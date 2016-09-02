#ifndef RENDERMODELBATCHKEY_H
#define RENDERMODELBATCHKEY_H

#include "renderer_global.h"
#include <QtOpenGL>
#include <QHash>

namespace NS_RENDERER
{
    class RENDERERSHARED_EXPORT RenderModelBatchKey
    {
    public:
        RenderModelBatchKey(quint16 shaderId, quint32 textureId, GLenum drawMode = GL_TRIANGLES, float drawWidth = 1.0f);
        RenderModelBatchKey(const RenderModelBatchKey &other);

        inline bool operator ==(const RenderModelBatchKey &other) const
        {
            return m_iShaderId == other.m_iShaderId &&
                    m_iTextureId == other.m_iTextureId &&
                    m_iDrawMode == other.m_iDrawMode &&
                    m_flDrawWidth == other.m_flDrawWidth;
        }

        quint16 shaderId() const;
        quint32 textureId() const;
        GLenum drawMode() const;
        float drawWidth() const;

    private:
        quint16     m_iShaderId;
        quint32     m_iTextureId;
        GLenum      m_iDrawMode;
        float       m_flDrawWidth;
    };
}

static uint qHash(const NS_RENDERER::RenderModelBatchKey &key, uint seed)
{
    QByteArray arr;

    {
        QDataStream stream(&arr, QIODevice::WriteOnly);
        stream << key.shaderId()
               << key.textureId()
               << key.drawMode()
               << key.drawWidth();
    }

    return qHash(arr, seed);
}

#endif // RENDERMODELBATCHKEY_H
