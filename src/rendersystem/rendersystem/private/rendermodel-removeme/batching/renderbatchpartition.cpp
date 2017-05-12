#include "renderbatchpartition.h"

#include "rendersystem/private/shaders/common/privateshaderdefs.h"
#include "rendersystem/private/opengl/openglerrors.h"
#include "rendersystem/private/opengl/openglhelpers.h"

namespace
{
    const quint32 SIZEOF_MATRIX_4X4 = 4 * 4 * sizeof(float);

    // Assumes destination is large enough to hold all required elements.
    template<typename T>
    void consolidate(const QVector<T> &source,
                     QVector<T> &destination,
                     int components,
                     int offset,
                     int stride)
    {
        if ( stride <= 0 )
        {
            stride = 1;
        }

        for ( int srcItem = 0, destItem = offset; srcItem < source.count(); srcItem += components, destItem += stride)
        {
            for ( int curComponent = 0; curComponent < components; ++curComponent )
            {
                destination[destItem + curComponent] = source[srcItem + curComponent];
            }
        }
    }

    inline quint32 maskFromNumberOfBits(int numBits)
    {
        return ~(static_cast<quint32>(0)) >> ((sizeof(quint32) * 8) - numBits);
    }

    inline quint8 bitsRequired(int maxValue)
    {
        for ( quint32 i = 0; i < sizeof(quint32) * 8; i++ )
        {
            if ( (1 << i) >= maxValue )
            {
                return i+1;
            }
        }

        return sizeof(quint32) * 8;
    }
}

RenderBatchPartition::RenderBatchPartition(const VertexFormat &vertexFormat,
                         int maxBatches,
                         int maxItemsPerBatch,
                         QOpenGLBuffer::UsagePattern usagePattern)
    : m_Buffers(usagePattern),
      m_Data(maxBatches, maxItemsPerBatch),
      m_VertexFormat(vertexFormat),
      m_BatchMetadata(),
      m_nDrawMode(GL_TRIANGLES),
      m_nVertexFloatsUploadedSoFar(0),
      m_nIndexIntsUploadedSoFar(0),
      m_pVertexBufferPointer(Q_NULLPTR),
      m_pIndexBufferPointer(Q_NULLPTR),
      m_nObjectIdMask(calculateObjectIdMask()),
      m_nCurrentObjectId(0)
{
    m_Buffers.create();
}

BufferDataContainer& RenderBatchPartition::data()
{
    return m_Data;
}

const BufferDataContainer& RenderBatchPartition::data() const
{
    return m_Data;
}

bool RenderBatchPartition::isFull() const
{
    return m_Data.isFull();
}

GLenum RenderBatchPartition::drawMode() const
{
    return m_nDrawMode;
}

void RenderBatchPartition::setDrawMode(GLenum mode)
{
    m_nDrawMode = mode;
}

void RenderBatchPartition::draw()
{
    m_Buffers.vertexBuffer().bind();
    m_Buffers.indexBuffer().bind();

    for ( int i = 0; i < m_BatchMetadata.count(); ++i )
    {
        drawBatch(i);
    }

    m_Buffers.vertexBuffer().release();
    m_Buffers.indexBuffer().release();
}

void RenderBatchPartition::drawBatch(int index)
{
    const quint32 matricesPerBatch = m_Data.numItemsPerBatch();
    const quint32 offsetInMatrices = index * matricesPerBatch;
    const quint32 offsetInBytes = offsetInMatrices * SIZEOF_MATRIX_4X4;

    quint32 lengthInMatrices = matricesPerBatch;
    if ( offsetInMatrices + lengthInMatrices > static_cast<quint32>(m_Data.batchItemCount()) )
    {
        lengthInMatrices = m_Data.batchItemCount() % matricesPerBatch;
    }

    const quint32 lengthInBytes = lengthInMatrices * SIZEOF_MATRIX_4X4;

    m_Buffers.uniformBuffer().bindRange(PrivateShaderDefs::LocalUniformBlockBindingPoint, offsetInBytes, lengthInBytes);

    const BatchMetadata& metadata = m_BatchMetadata.at(index);
    const quint32 indicesToDraw = metadata.indexLengthBytes / sizeof(quint32);
    const quint32 indexByteOffset = metadata.indexOffsetBytes;

    GL_CURRENT_F;
    GLTRY(f->glDrawElements(m_nDrawMode, indicesToDraw, GL_UNSIGNED_INT, reinterpret_cast<GLvoid*>(indexByteOffset)));

    m_Buffers.uniformBuffer().release();
}

bool RenderBatchPartition::ensureUploaded()
{
    if ( !m_Buffers.isCreated() )
    {
        return false;
    }

    if ( !anyDirtyObjectData() )
    {
        return true;
    }

    // We don't do any clever optimisations here - if anything is dirty,
    // reallocate the buffer and re-upload everything.
    // Profiling may show whether this approach is too slow later on.

    m_BatchMetadata.clear();
    markAllObjectsDirty();
    m_Buffers.setBatchSize(m_Data.numItemsPerBatch());

    m_nVertexFloatsUploadedSoFar = 0;
    m_nIndexIntsUploadedSoFar = 0;

    if ( !allocateAndMapBuffers() )
    {
        unmapBuffers();
        return false;
    }

    for ( int object = 0; object < m_Data.batchItemCount(); ++object )
    {
        uploadData(object);
    }

    unmapBuffers();
    return true;
}

void RenderBatchPartition::uploadBatchData(int batchIndex)
{
    // Leave the lengths as 0 until we know how much data has been uploaded in this batch.
    m_BatchMetadata.append(BatchMetadata(m_nVertexFloatsUploadedSoFar * sizeof(float), 0,
                                         m_nIndexIntsUploadedSoFar * sizeof(quint32), 0));

    for ( int batchItemIndex = 0;
          batchItemIndex < m_Data.numItemsPerBatch() && (batchIndex + batchItemIndex) < m_Data.batchItemCount();
          ++batchItemIndex )
    {
        uploadData(batchIndex + batchItemIndex);
    }

    // Now update the lengths.
    BatchMetadata& metadata = m_BatchMetadata.last();
    metadata.vertexLengthBytes = (m_nVertexFloatsUploadedSoFar * sizeof(float)) - metadata.vertexOffsetBytes;
    metadata.indexLengthBytes = (m_nIndexIntsUploadedSoFar * sizeof(quint32)) - metadata.indexOffsetBytes;
}

void RenderBatchPartition::uploadData(int index)
{
    QSharedPointer<ObjectSectionGeometryData> objectGeometry = m_Data.geometryData(index);

    GLTRY(m_Buffers.uniformBuffer().write(index * SIZEOF_MATRIX_4X4, objectGeometry->modelToWorldMatrix().constData(), SIZEOF_MATRIX_4X4));

    QVector<float> vertexData;
    QVector<quint32> indexData;
    m_nCurrentObjectId = index % m_Data.numItemsPerBatch();

    consolidateVetexData(objectGeometry, vertexData);
    consolidateIndexData(objectGeometry, indexData);

    quint32 indexIncrement = m_nVertexFloatsUploadedSoFar / m_VertexFormat.totalVertexComponents();

    uploadVertexData(vertexData, m_nVertexFloatsUploadedSoFar);
    uploadIndexData(indexData, m_nIndexIntsUploadedSoFar, indexIncrement);

    m_nVertexFloatsUploadedSoFar += vertexData.count();
    m_nIndexIntsUploadedSoFar += indexData.count();

    objectGeometry->markAsCleaned();
}

bool RenderBatchPartition::allocateAndMapBuffers()
{
    if ( !m_Buffers.vertexBuffer().bind() || !m_Buffers.indexBuffer().bind() )
    {
        return false;
    }

    quint32 vertexBufferSize = totalRequiredVertexBytes();
    m_Buffers.vertexBuffer().allocate(vertexBufferSize);
    m_pVertexBufferPointer = reinterpret_cast<float*>(m_Buffers.vertexBuffer().map(QOpenGLBuffer::WriteOnly));

    quint32 indexBufferSize = totalRequiredIndexBytes();
    m_Buffers.indexBuffer().allocate(indexBufferSize);
    m_pIndexBufferPointer = reinterpret_cast<quint32*>(m_Buffers.indexBuffer().map(QOpenGLBuffer::WriteOnly));

    OpenGLUniformBuffer& uniformBuffer = m_Buffers.uniformBuffer();

    // Warning: The specific order of these commands may be important.
    // UBOs are finnicky.

    GLTRY(uniformBuffer.bind());
    GLTRY(uniformBuffer.allocate(m_Data.batchItemCount() * SIZEOF_MATRIX_4X4));
    GLTRY(uniformBuffer.release());
    GLTRY(uniformBuffer.bindToIndex(PrivateShaderDefs::LocalUniformBlockBindingPoint));

    return m_pVertexBufferPointer && m_pIndexBufferPointer;
}

void RenderBatchPartition::unmapBuffers()
{
    m_Buffers.vertexBuffer().unmap();
    m_Buffers.indexBuffer().unmap();

    m_Buffers.vertexBuffer().release();
    m_Buffers.indexBuffer().release();
    m_Buffers.uniformBuffer().release();
}

void RenderBatchPartition::uploadVertexData(const QVector<float> &vertexData, int offsetInFloats)
{
    float* bufferPointer = m_pVertexBufferPointer + offsetInFloats;
    memcpy(bufferPointer, vertexData.constData(), vertexData.count() * sizeof(float));
}

void RenderBatchPartition::uploadIndexData(const QVector<quint32> &indexData, int offsetInInts, quint32 indexIncrement)
{
    quint32* bufferPointer = m_pIndexBufferPointer + offsetInInts;

    for ( int i = 0; i < indexData.count(); ++i )
    {
        bufferPointer[i] = indexData.at(i) + indexIncrement;
    }
}

bool RenderBatchPartition::anyDirtyObjectData() const
{
    for ( int i = 0; i < m_Data.batchItemCount(); ++i )
    {
        if ( m_Data.geometryData(i)->isDirty() )
        {
            return true;
        }
    }

    return false;
}

void RenderBatchPartition::markAllObjectsDirty()
{
    for ( int i = 0; i < m_Data.batchItemCount(); ++i )
    {
        m_Data.geometryData(i)->markAsDirty();
    }
}

quint32 RenderBatchPartition::totalRequiredVertexBytes() const
{
    quint32 bytes = 0;

    for ( int i = 0; i < m_Data.batchItemCount(); ++i )
    {
        bytes += m_Data.geometryData(i)->computeTotalVertexBytes();
    }

    return bytes;
}

quint32 RenderBatchPartition::totalRequiredIndexBytes() const
{
    quint32 bytes = 0;

    for ( int i = 0; i < m_Data.batchItemCount(); ++i )
    {
        bytes += m_Data.geometryData(i)->computeTotalIndexBytes();
    }

    return bytes;
}

void RenderBatchPartition::consolidateVetexData(const QSharedPointer<ObjectSectionGeometryData> &objectGeometry,
                                                     QVector<float> &vertexData)
{
    quint32 totalVertexDataBytes = objectGeometry->computeTotalVertexBytes();
    int totalVertexComponents = m_VertexFormat.totalVertexComponents();

    vertexData.resize(totalVertexDataBytes);

    int beginOffset = 0;

    consolidate<float>(objectGeometry->positions(),
                       vertexData,
                       m_VertexFormat.positionComponents(),
                       beginOffset,
                       totalVertexComponents,
                       true);

    beginOffset += m_VertexFormat.positionComponents();

    consolidate<float>(objectGeometry->normals(),
                       vertexData,
                       m_VertexFormat.normalComponents(),
                       beginOffset,
                       totalVertexComponents);

    beginOffset += m_VertexFormat.normalComponents();

    consolidate<float>(objectGeometry->colors(),
                       vertexData,
                       m_VertexFormat.colorComponents(),
                       beginOffset,
                       totalVertexComponents);

    beginOffset += m_VertexFormat.colorComponents();

    consolidate<float>(objectGeometry->textureCoordinates(),
                       vertexData,
                       m_VertexFormat.textureCoordinateComponents(),
                       beginOffset,
                       totalVertexComponents);
}

void RenderBatchPartition::consolidateIndexData(const QSharedPointer<ObjectSectionGeometryData> &objectGeometry,
                                                     QVector<quint32> &indexData)
{
    quint32 totalIndexBytes = objectGeometry->computeTotalIndexBytes();
    indexData.resize(totalIndexBytes);

    consolidate<quint32>(objectGeometry->indices(),
                         indexData,
                         1,
                         0,
                         1);
}

quint32 RenderBatchPartition::calculateObjectIdMask() const
{
    return maskFromNumberOfBits(bitsRequired(m_Data.numItemsPerBatch()));
}