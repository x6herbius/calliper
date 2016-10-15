#ifndef TEXTURESTORE_H
#define TEXTURESTORE_H

#include "model_global.h"
#include <QHash>
#include "opengl/opengltexture.h"
#include "functors/itextureretrievalfunctor.h"

namespace NS_MODEL
{
    class MODELSHARED_EXPORT TextureStore : public NS_RENDERER::ITextureRetrievalFunctor
    {
    public:
        TextureStore();
        ~TextureStore();

        virtual NS_RENDERER::OpenGLTexturePointer operator ()(quint64 textureId) const override;
        NS_RENDERER::OpenGLTexturePointer getTexture(quint32 textureId) const;
        NS_RENDERER::OpenGLTexturePointer createTexture(const QString &path, QOpenGLTexture::Target target = QOpenGLTexture::Target2D);
        quint32 getTextureId(const QString &path) const;
        NS_RENDERER::OpenGLTexturePointer getTexture(const QString &path) const;

    private:
        quint32 acquireNextTextureId();

        quint32 m_iNextTextureId;
        QHash<quint32, NS_RENDERER::OpenGLTexturePointer> m_TextureTable;
        QHash<QString, quint32> m_TexturePathTable;
    };
}

#endif // TEXTURESTORE_H
