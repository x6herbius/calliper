#include "rendermodelbatch.h"
#include "openglerrors.h"
#include "openglhelpers.h"
#include "shaderdefs.h"
#include "ishaderspec.h"

namespace NS_RENDERER
{
    RenderModelBatch::RenderModelBatch(QOpenGLBuffer::UsagePattern usagePattern, QObject *parent)
        : QObject(parent),
          m_iVAOID(0),
          m_iUsagePattern(usagePattern),
          m_bCreated(false),
          m_GlVertexBuffer(QOpenGLBuffer::VertexBuffer),
          m_GlIndexBuffer(QOpenGLBuffer::IndexBuffer),
          m_pShaderSpec(NULL),
          m_bDataStale(false)
    {
    }

    RenderModelBatch::~RenderModelBatch()
    {
        destroy();
    }

    GLuint RenderModelBatch::vaoHandle() const
    {
        return m_iVAOID;
    }

    bool RenderModelBatch::create()
    {
        if ( m_bCreated )
            return true;

        GL_CURRENT_F;

        GLTRY(f->glGenVertexArrays(1, &m_iVAOID));
        GLTRY(f->glBindVertexArray(m_iVAOID));

        m_GlVertexBuffer.setUsagePattern(m_iUsagePattern);
        m_GlIndexBuffer.setUsagePattern(m_iUsagePattern);

        m_bCreated = m_GlVertexBuffer.create() && m_GlIndexBuffer.create();
        return m_bCreated;
    }

    void RenderModelBatch::destroy()
    {
        if ( !m_bCreated )
            return;

        GL_CURRENT_F;

        m_GlVertexBuffer.destroy();
        m_GlIndexBuffer.destroy();

        GLTRY(f->glDeleteVertexArrays(1, &m_iVAOID));

        m_bCreated = false;
    }

    void RenderModelBatch::addItem(const RenderModelBatchParams &params)
    {
        if ( !m_pShaderSpec )
            return;

        if ( m_Items.count() >= m_pShaderSpec->maxBatchedItems() )
            return;

        int newVertexOffset = 0;
        if ( !m_Items.isEmpty() )
        {
            newVertexOffset = m_Items.last().vertexOffset() + m_Items.last().vertexCount();
        }

        int oldVertexCount = m_LocalPositionBuffer.count() / m_pShaderSpec->positionComponents();
        int newVertexCount = params.vertexCount();

        resizeAllBuffers(oldVertexCount + newVertexCount);
        copyInData(params, oldVertexCount);

        int newIndexOffset = 0;
        if ( !m_Items.isEmpty() )
        {
            newIndexOffset = m_Items.last().indexOffset() + m_Items.last().indexCount();
        }

        int newIndexCount = params.indexCount();
        m_LocalIndexBuffer.resize(m_LocalIndexBuffer.count() + newIndexCount);

        memcpy(m_LocalIndexBuffer.data() + newIndexOffset,
               params.indices(),
               newIndexCount * sizeof(quint32));

        m_ModelToWorldMatrices.append(params.modelToWorldMatrix());

        m_Items.append(RenderModelBatchItem(newVertexOffset, newVertexCount, newIndexOffset, newIndexCount));
        m_bDataStale = true;
    }

    void RenderModelBatch::copyInData(const RenderModelBatchParams &params, int vertexCount)
    {
        // Construct some padding zeroes for unspecified data.
        // We need to set this to match the attribute  with the
        // largest number of components.
        QVector<float> padding;
        if ( params.someAttributesUnspecified() )
        {
            int components = maxComponentsFromVertexSpec();
            padding.insert(0, params.vertexCount() * components, 0);
        }

        memcpy(m_LocalPositionBuffer.data() + (vertexCount * m_pShaderSpec->positionComponents()),
               params.positions(),
               params.vertexCount() * m_pShaderSpec->positionComponents() * sizeof(float));

        memcpy(m_LocalNormalBuffer.data() + (vertexCount * m_pShaderSpec->normalComponents()),
               params.hasNormals() ? params.normals() : padding.constData(),
               params.vertexCount() * m_pShaderSpec->normalComponents() * sizeof(float));

        memcpy(m_LocalColorBuffer.data() + (vertexCount * m_pShaderSpec->colorComponents()),
               params.hasColors() ? params.colors() : padding.constData(),
               params.vertexCount() * m_pShaderSpec->colorComponents() * sizeof(float));

        memcpy(m_LocalTextureCoordinateBuffer.data() + (vertexCount * m_pShaderSpec->textureCoordinateComponents()),
               params.hasTextureCoordinates() ? params.textureCoordinates() : padding.constData(),
               params.vertexCount() * m_pShaderSpec->textureCoordinateComponents() * sizeof(float));
    }

    void RenderModelBatch::resizeAllBuffers(int numVertices)
    {
        Q_ASSERT_X(m_pShaderSpec, Q_FUNC_INFO, "Shader spec required!");

        m_LocalPositionBuffer.resize(numVertices * m_pShaderSpec->positionComponents());
        m_LocalNormalBuffer.resize(numVertices * m_pShaderSpec->normalComponents());
        m_LocalColorBuffer.resize(numVertices * m_pShaderSpec->colorComponents());
        m_LocalTextureCoordinateBuffer.resize(numVertices * m_pShaderSpec->textureCoordinateComponents());
    }

    int RenderModelBatch::maxComponentsFromVertexSpec() const
    {
        int components = 0;

        if ( m_pShaderSpec->positionComponents() > components )
            components = m_pShaderSpec->positionComponents();

        if ( m_pShaderSpec->normalComponents() > components )
            components = m_pShaderSpec->normalComponents();

        if ( m_pShaderSpec->colorComponents() > components )
            components = m_pShaderSpec->colorComponents();

        if ( m_pShaderSpec->textureCoordinateComponents() > components )
            components = m_pShaderSpec->textureCoordinateComponents();

        return components;
    }

    int RenderModelBatch::itemCount() const
    {
        return m_Items.count();
    }

    const IShaderSpec* RenderModelBatch::shaderSpec() const
    {
        return m_pShaderSpec;
    }

    void RenderModelBatch::setShaderSpec(const IShaderSpec *spec)
    {
        m_pShaderSpec = spec;
    }

    void RenderModelBatch::upload(bool force)
    {
        if ( force || m_bDataStale )
        {
            uploadVertexData();

            m_GlIndexBuffer.bind();
            m_GlIndexBuffer.allocate(m_LocalIndexBuffer.constData(), m_LocalIndexBuffer.count() * sizeof(quint32));

            m_bDataStale = false;
        }
    }

    bool RenderModelBatch::needsUpload() const
    {
        return m_bDataStale;
    }

    void RenderModelBatch::uploadVertexData()
    {
        int size = (m_LocalPositionBuffer.count()
                    + m_LocalNormalBuffer.count()
                    + m_LocalColorBuffer.count()
                    + m_LocalTextureCoordinateBuffer.count())
                * sizeof(float);

        m_GlVertexBuffer.bind();
        m_GlVertexBuffer.allocate(size);

        int offset = 0;
        writeToGlVertexBuffer(m_LocalPositionBuffer, offset);
        writeToGlVertexBuffer(m_LocalNormalBuffer, offset);
        writeToGlVertexBuffer(m_LocalColorBuffer, offset);
        writeToGlVertexBuffer(m_LocalTextureCoordinateBuffer, offset);
    }

    void RenderModelBatch::writeToGlVertexBuffer(const QVector<float> &buffer, int &offset)
    {
        m_GlVertexBuffer.write(offset, buffer.constData(), buffer.count() * sizeof(float));
        offset += buffer.count() * sizeof(float);
    }

    int RenderModelBatch::localPositionCount() const
    {
        return m_LocalPositionBuffer.count();
    }

    int RenderModelBatch::localNormalCount() const
    {
        return m_LocalNormalBuffer.count();
    }

    int RenderModelBatch::localColorCount() const
    {
        return m_LocalColorBuffer.count();
    }

    int RenderModelBatch::localTextureCoordinateCount() const
    {
        return m_LocalTextureCoordinateBuffer.count();
    }

    int RenderModelBatch::localIndexCount() const
    {
        return m_LocalIndexBuffer.count();
    }

    void RenderModelBatch::clearItems()
    {
        m_LocalPositionBuffer.clear();
        m_LocalNormalBuffer.clear();
        m_LocalColorBuffer.clear();
        m_LocalTextureCoordinateBuffer.clear();
        m_LocalIndexBuffer.clear();
        m_ModelToWorldMatrices.clear();
        m_Items.clear();

        m_bDataStale = true;
    }
}