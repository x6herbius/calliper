#include "geometryconsolidator.h"

namespace
{
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

GeometryConsolidator::GeometryConsolidator(GeometryDataContainer &data, OpenGLShaderProgram *shader)
    : m_GeometryDataContainer(data),
      m_pShader(shader),
      m_VertexData(),
      m_IndexData(),
      m_OffsetData(),
      m_nItemsPerBatch(0),
      m_nCurrentObjectId(0),
      m_nObjectIdMask(0)
{

}

OpenGLShaderProgram* GeometryConsolidator::shader() const
{
    return m_pShader;
}

void GeometryConsolidator::setShader(OpenGLShaderProgram *shader)
{
    if ( shader == m_pShader )
    {
        return;
    }

    m_pShader = shader;
    clear();
}

void GeometryConsolidator::clear()
{
    m_VertexData.clear();
    m_IndexData.clear();
    m_OffsetData.clear();
    m_nItemsPerBatch = m_pShader ? m_pShader->maxBatchedItems() : 0;
    m_nCurrentObjectId = 0;

    if ( m_nItemsPerBatch > 1 )
    {
        m_nObjectIdMask = maskFromNumberOfBits(bitsRequired(m_nItemsPerBatch));
    }
    else
    {
        m_nObjectIdMask = 0;
    }
}

const QVector<float>& GeometryConsolidator::vertexData() const
{
    return m_VertexData;
}

const QVector<quint32>& GeometryConsolidator::indexData() const
{
    return m_IndexData;
}

void GeometryConsolidator::consolidate()
{
    if ( !m_pShader )
    {
        return;
    }

    clear();

    for ( GeometryDataContainer::ConstIterator it = m_GeometryDataContainer.constBegin();
          it != m_GeometryDataContainer.constEnd();
          ++it )
    {
        GeometryDataContainer::GeometryDataPointer geometry = it.value();
        consolidate(geometry);

        m_nCurrentObjectId = (m_nCurrentObjectId + 1) % m_nItemsPerBatch;
    }
}

void GeometryConsolidator::consolidate(const QSharedPointer<GeometryData> &geometry)
{
    m_OffsetData.append(ObjectOffsets());
    consolidateVertices(geometry);
    consolidateIndices(geometry);
}

void GeometryConsolidator::consolidateVertices(const QSharedPointer<GeometryData> &geometry)
{
    const VertexFormat vertexFormat = m_pShader->vertexFormat();
    ObjectOffsets& offsets = m_OffsetData.last();
    offsets.vertexOffsetFloats = m_VertexData.count();
    offsets.vertexCountFloats = geometry->computeTotalVertexBytes(vertexFormat) / sizeof(float);

    quint32 beginOffsetInFloats = offsets.vertexOffsetFloats;

    m_VertexData.resize(m_VertexData.count() + offsets.vertexCountFloats);

    consolidateVertices(geometry->positions(),
                        vertexFormat.positionComponents(),
                        beginOffsetInFloats,
                        m_nObjectIdMask > 0);

    beginOffsetInFloats += vertexFormat.positionComponents();

    consolidateVertices(geometry->normals(),
                        vertexFormat.normalComponents(),
                        beginOffsetInFloats);

    beginOffsetInFloats += vertexFormat.normalComponents();

    consolidateVertices(geometry->colors(),
                        vertexFormat.colorComponents(),
                        beginOffsetInFloats);

    beginOffsetInFloats += vertexFormat.colorComponents();

    consolidateVertices(geometry->textureCoordinates(),
                        vertexFormat.textureCoordinateComponents(),
                        beginOffsetInFloats);
}

void GeometryConsolidator::consolidateVertices(const QVector<QVector4D> &source,
                                               int components,
                                               quint32 offset,
                                               bool encodeId)
{
    int stride = m_pShader->vertexFormat().totalVertexComponents();

    for ( int srcItem = 0, destItem = offset;
          srcItem < source.count();
          ++srcItem, destItem += stride)
    {
        const QVector4D& vec = source[srcItem];

        for ( int curComponent = 0; curComponent < components; ++curComponent )
        {
            float value = vec[curComponent];

            if ( encodeId && curComponent == components - 1 )
            {
                quint32* valueAsInt = reinterpret_cast<quint32*>(&value);
                *valueAsInt |= (static_cast<quint32>(m_nCurrentObjectId) & m_nObjectIdMask);
            }

            m_VertexData[destItem + curComponent] = value;
        }
    }
}

void GeometryConsolidator::consolidateIndices(const QSharedPointer<GeometryData> &geometry)
{
    ObjectOffsets& offsets = m_OffsetData.last();
    offsets.indexOffsetInts = m_IndexData.count();
    offsets.indexCountInts = geometry->computeTotalIndexBytes() / sizeof(quint32);

    const VertexFormat vertexFormat = m_pShader->vertexFormat();
    const quint32 indexIncrement = offsets.vertexCountFloats / vertexFormat.totalVertexComponents();

    m_IndexData.resize(m_IndexData.count() + offsets.indexCountInts);
    consolidateIndices(geometry->indices(), offsets.indexOffsetInts, indexIncrement);
}

void GeometryConsolidator::consolidateIndices(const QVector<quint32> &indices, quint32 offsetInInts, quint32 indexIncrement)
{
    for ( int i = 0; i < indices.count(); ++i )
    {
        m_IndexData[offsetInInts + i] = indices[i] + indexIncrement;
    }
}
