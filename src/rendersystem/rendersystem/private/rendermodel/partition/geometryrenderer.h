#ifndef GEOMETRYRENDERER_H
#define GEOMETRYRENDERER_H

#include <QException>

#include "geometryoffsettable.h"
#include "openglbuffercollection.h"

#include "rendersystem/private/rendermodel/rendermodelcontext.h"
#include "rendersystem/private/shaders/common/privateshaderdefs.h"
#include "rendersystem/interface-classes/store/publicstoredefs.h"

class OpenGLShaderProgram;

class GeometryRenderer
{
public:
    GeometryRenderer(const RenderModelContext& context,
                     RenderSystem::PublicStoreDefs::MaterialId materialId,
                     GeometryOffsetTable& offsets,
                     OpenGLBufferCollection& openGLBuffers);

    void draw();

    GLenum drawMode() const;
    void setDrawMode(GLenum mode);

private:
    class InternalException : public QException
    {
    public:
        const QString error;

        InternalException(const QString& errorString)
            : error(errorString)
        {
        }

        void raise() const
        {
            throw *this;
        }

        InternalException *clone() const
        {
            return new InternalException(*this);
        }
    };

    void draw_x(int firstItem, int lastItem);
    void bindBuffers_x(int firstItem, int lastItem);
    void releaseBuffers();

    const RenderModelContext& m_Context;
    const RenderSystem::PublicStoreDefs::MaterialId m_nMaterialId;
    GeometryOffsetTable& m_OffsetTable;
    OpenGLBufferCollection& m_OpenGLBuffers;

    int m_nItemsPerBatch;
    OpenGLShaderProgram* m_pCurrentShader;
    GLenum m_nDrawMode;
};

#endif // GEOMETRYRENDERER_H