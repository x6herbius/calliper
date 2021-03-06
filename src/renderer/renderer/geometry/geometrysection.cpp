#include "geometrysection.h"
#include "renderer/materials/rendermaterial.h"

namespace
{
    class QColorWrapper
    {
    public:
        QColorWrapper(const QColor &col)
            : m_Color(col)
        {
        }

        qreal operator [](int index) const
        {
            switch (index)
            {
                case 0:
                    return m_Color.redF();
                case 1:
                    return m_Color.greenF();
                case 2:
                    return m_Color.blueF();
                case 3:
                    return m_Color.alphaF();
                default:
                    Q_ASSERT_X(false, Q_FUNC_INFO, "Index out of range");
                    return 0.0f;
            }
        }

    private:
        const QColor& m_Color;
    };

    // Component count is how many components per attribute the vertex format expects.
    // eg. 4 floats for a position.
    // Anything beyond what the input vector can provide will be left as 0.
    template<typename T>
    void append(QVector<float> &list, const T &vec, int vecLength, int componentCount)
    {
        if ( componentCount < 1 || vecLength < 1 )
            return;

        int count = list.count();
        list.resize(count + componentCount);

        float* data = list.data() + count;
        int min = qMin(componentCount, vecLength);
        for ( int i = 0; i < min; i++ )
        {
                data[i] = vec[i];
        }
    }

    int vertexFormatComponents(const Renderer::VertexFormat &format, Renderer::GeometrySection::AttributeType att)
    {
        using namespace Renderer;

        switch (att)
        {
            case GeometrySection::PositionAttribute:
                return format.positionComponents();

            case GeometrySection::NormalAttribute:
                return format.normalComponents();

            case GeometrySection::ColorAttribute:
                return format.colorComponents();

            case GeometrySection::TextureCoordinateAttribute:
                return format.textureCoordinateComponents();

            default:
                Q_ASSERT_X(false, Q_FUNC_INFO, "Unrecognised attribute");
                return 0;
        }
    }
}

namespace Renderer
{
    GeometrySection::GeometrySection(const RenderFunctorGroup& renderFunctors,
                                     BaseShaderPalette* shaderPalette,
                                     quint32 materialId,
                                     const QMatrix4x4 modelToWorldMatrix)
        : m_iDrawMode(GL_TRIANGLES),
          m_flDrawWidth(1),
          m_RenderFunctors(renderFunctors),
          m_pShaderPalette(shaderPalette),
          m_iMaterialId(materialId),
          m_matModelToWorld(modelToWorldMatrix),
          m_pShader(getShader()),
          m_VertexFormat(m_pShader ? m_pShader->vertexFormat() : VertexFormat(0,0,0,0))
    {
        init();
    }

    void GeometrySection::init()
    {
        for ( int i = 0; i < AttributeTypeCount; i++ )
        {
            m_Attributes.append(QVector<float>());
        }
    }

    void GeometrySection::add(AttributeType att, const float *data, int count)
    {
        append<const float*>(m_Attributes[att], data, count,
                             vertexFormatComponents(m_VertexFormat, att));
    }

    void GeometrySection::addPosition(const QVector3D &pos)
    {
        append<QVector3D>(m_Attributes[PositionAttribute], pos, 3,
                          vertexFormatComponents(m_VertexFormat, PositionAttribute));
    }

    void GeometrySection::addPosition(const QVector4D &pos)
    {
        append<QVector4D>(m_Attributes[PositionAttribute], pos, 4,
                          vertexFormatComponents(m_VertexFormat, PositionAttribute));
    }

    void GeometrySection::addPositions(const QVector<QVector3D> &list)
    {
        QVector<float>& posList = m_Attributes[PositionAttribute];
        posList.reserve(posList.count() + (list.count() * 3));

        foreach (const QVector3D &vec, list)
        {
            append<QVector3D>(posList, vec, 3,
                              vertexFormatComponents(m_VertexFormat, PositionAttribute));
        }
    }

    void GeometrySection::addNormal(const QVector3D &vec)
    {
        append<QVector3D>(m_Attributes[NormalAttribute], vec, 3,
                          vertexFormatComponents(m_VertexFormat, NormalAttribute));
    }

    void GeometrySection::addColor(const QColor &col)
    {
        append<QColorWrapper>(m_Attributes[ColorAttribute], QColorWrapper(col), 4,
                          vertexFormatComponents(m_VertexFormat, ColorAttribute));
    }

    void GeometrySection::addTextureCoordinate(const QVector2D &coord)
    {
        append<QVector2D>(m_Attributes[TextureCoordinateAttribute], coord, 2,
                          vertexFormatComponents(m_VertexFormat, TextureCoordinateAttribute));
    }

    int GeometrySection::floatCount(AttributeType att) const
    {
        return m_Attributes.at(att).count();
    }

    int GeometrySection::indexCount() const
    {
        return m_Indices.count();
    }

    void GeometrySection::addIndex(quint32 idx)
    {
        m_Indices.append(idx);
    }

    void GeometrySection::addIndexTriangle(quint32 i0, quint32 i1, quint32 i2)
    {
        m_Indices.reserve(m_Indices.count() + 3);
        m_Indices.append(i0);
        m_Indices.append(i1);
        m_Indices.append(i2);
    }

    void GeometrySection::addIndexLine(quint32 i0, quint32 i1)
    {
        m_Indices.reserve(m_Indices.count() + 2);
        m_Indices.append(i0);
        m_Indices.append(i1);
    }

    void GeometrySection::addVertex(const Vertex3D &vertex)
    {
        addPosition(vertex.position());
        addNormal(vertex.normal());
        addColor(vertex.color());
        addTextureCoordinate(vertex.textureCoordinate());
    }

    void GeometrySection::addVertex(const Vertex3DValueRef &vertex)
    {
        addPosition(vertex.position());
        addNormal(vertex.normal());
        addColor(vertex.color());
        addTextureCoordinate(vertex.textureCoordinate());
    }

    const float* GeometrySection::vertexConstData(AttributeType type) const
    {
        return m_Attributes.at(type).constData();
    }

    const quint32* GeometrySection::indexConstData() const
    {
        return m_Indices.constData();
    }

    const QVector<float>& GeometrySection::vertexConstVector(AttributeType type) const
    {
        return m_Attributes.at(type);
    }

    const QVector<quint32>& GeometrySection::indexConstVector() const
    {
        return m_Indices;
    }

    int GeometrySection::attributeCount(AttributeType att) const
    {
        return m_Attributes[att].count() / vertexFormatComponents(m_VertexFormat, att);
    }

    bool GeometrySection::hasNormals() const
    {
        return !m_Attributes[NormalAttribute].isEmpty();
    }

    bool GeometrySection::hasColors() const
    {
        return !m_Attributes[ColorAttribute].isEmpty();
    }

    bool GeometrySection::hasTextureCoordinates() const
    {
        return !m_Attributes[TextureCoordinateAttribute].isEmpty();
    }

    void GeometrySection::addPositions(const float *data, int count)
    {
        add(PositionAttribute, data, count);
    }

    int GeometrySection::consolidate(QVector<float> &positions, QVector<float> &normals, QVector<float> &colors,
                                      QVector<float> &textureCoordinates, QVector<quint32> &indices) const
    {
        positions.append(vertexConstVector(GeometrySection::PositionAttribute));
        normals.append(vertexConstVector(GeometrySection::NormalAttribute));
        colors.append(vertexConstVector(GeometrySection::ColorAttribute));
        textureCoordinates.append(vertexConstVector(GeometrySection::TextureCoordinateAttribute));

        // We need to renumber the indices from section-specific to global.
        int oldIndexCount = indices.count();
        indices.resize(oldIndexCount + indexCount());
        quint32* indexData = indices.data() + oldIndexCount;
        const quint32* indexSource = indexConstData();

        for ( int i = 0; i < indexCount(); i++ )
        {
            indexData[i] = oldIndexCount + indexSource[i];
        }

        return indexCount();
    }

    IShaderRetrievalFunctor* GeometrySection::shaderFunctor() const
    {
        return m_RenderFunctors.shaderFunctor;
    }

    ITextureRetrievalFunctor* GeometrySection::textureFunctor() const
    {
        return m_RenderFunctors.textureFunctor;
    }

    IMaterialRetrievalFunctor* GeometrySection::materialFunctor() const
    {
        return m_RenderFunctors.materialFunctor;
    }

    quint16 GeometrySection::shaderId() const
    {
        return getShader()->shaderStoreId();
    }

    quint32 GeometrySection::materialId() const
    {
        return m_iMaterialId;
    }

    void GeometrySection::setMaterialId(quint32 id)
    {
        m_iMaterialId = id;
    }

    GLenum GeometrySection::drawMode() const
    {
        return m_iDrawMode;
    }

    void GeometrySection::setDrawMode(GLenum mode)
    {
        m_iDrawMode = mode;
    }

    float GeometrySection::drawWidth() const
    {
        return m_flDrawWidth;
    }

    void GeometrySection::setDrawWidth(float width)
    {
        m_flDrawWidth = width;
    }

    const QMatrix4x4& GeometrySection::modelToWorldMatrix() const
    {
        return m_matModelToWorld;
    }

    void GeometrySection::setModelToWorldMatrix(const QMatrix4x4 &mat)
    {
        m_matModelToWorld = mat;
    }

    bool GeometrySection::isEmpty() const
    {
        // Positions considered canonical for vertex data count.
        return attributeCount(PositionAttribute) < 1 && indexCount() < 1;
    }

    void GeometrySection::clearAttribute(AttributeType att)
    {
        m_Attributes[att].clear();
    }

    VertexFormat GeometrySection::vertexFormat() const
    {
        return m_VertexFormat;
    }

    OpenGLShaderProgram* GeometrySection::getShader() const
    {
        RenderMaterialPointer material = (*materialFunctor())(m_iMaterialId);
        ShaderDefs::ShaderTechnique shaderTechnique = material->shaderTechnique();
        quint16 shaderId = m_pShaderPalette->shader(shaderTechnique);
        return (*shaderFunctor())(shaderId);
    }
}
