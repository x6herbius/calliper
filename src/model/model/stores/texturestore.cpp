#include "texturestore.h"
#include <QtDebug>

namespace Model
{
    Q_LOGGING_CATEGORY(lcTextureStore, "Model.TextureStore")

    TextureStore::TextureStore()
        : m_iNextTextureId(1),
          m_pDefaultTexture(Renderer::OpenGLTexturePointer::create(0, QOpenGLTexture::Target2D))
    {

    }

    TextureStore::~TextureStore()
    {
        using namespace Renderer;
        foreach ( OpenGLTexturePointer tex, m_TextureTable.values() )
        {
            tex->destroy();
        }
    }

    quint32 TextureStore::acquireNextTextureId()
    {
        Q_ASSERT_X(m_iNextTextureId + 1 != 0, Q_FUNC_INFO, "Texture ID counter overflow!");
        return m_iNextTextureId++;
    }

    Renderer::OpenGLTexturePointer TextureStore::getTexture(quint32 textureId) const
    {
        return m_TextureTable.value(textureId, m_pDefaultTexture);
    }

    Renderer::OpenGLTexturePointer TextureStore::operator ()(quint32 textureId) const
    {
        return getTexture(textureId);
    }

    Renderer::OpenGLTexturePointer TextureStore::createTextureFromFile(const QString &path)
    {
        using namespace Renderer;

        if ( m_TexturePathTable.contains(path) )
        {
            return getTexture(m_TexturePathTable.value(path));
        }

        OpenGLTexturePointer texture = OpenGLTexturePointer::create(acquireNextTextureId(), QImage(path).mirrored());
        texture->create();
        processCreatedTexture(texture, path);
        return texture;
    }

    Renderer::OpenGLTexturePointer TextureStore::createEmptyTexture(const QString &path)
    {
        using namespace Renderer;

        if ( m_TexturePathTable.contains(path) )
        {
            return getTexture(m_TexturePathTable.value(path));
        }

        OpenGLTexturePointer texture = OpenGLTexturePointer::create(acquireNextTextureId(), QOpenGLTexture::Target2D);
        processCreatedTexture(texture, path);
        return texture;
    }

    void TextureStore::processCreatedTexture(const Renderer::OpenGLTexturePointer &texture, const QString& path)
    {
        texture->setPath(path);
        m_TextureTable.insert(texture->textureStoreId(), texture);
        m_TexturePathTable.insert(path, texture->textureStoreId());
    }

    quint32 TextureStore::getTextureId(const QString &path) const
    {
        return m_TexturePathTable.value(path, 0);
    }

    Renderer::OpenGLTexturePointer TextureStore::getTexture(const QString &path) const
    {
        return getTexture(getTextureId(path));
    }

    void TextureStore::destroyTexture(quint32 textureId)
    {
        using namespace Renderer;

        if ( !m_TextureTable.contains(textureId) )
            return;

        OpenGLTexturePointer texture = getTexture(textureId);

        m_TextureTable.remove(textureId);
        m_TexturePathTable.remove(texture->path());
        texture->destroy();
    }

    void TextureStore::destroyTexture(const QString &path)
    {
        destroyTexture(getTextureId(path));
    }

    Renderer::OpenGLTexturePointer TextureStore::defaultTexture() const
    {
        return m_pDefaultTexture;
    }

    void TextureStore::setDefaultTextureFromFile(const QString &path)
    {
        QImage image(path);
        if ( image.isNull() )
        {
            qCWarning(lcTextureStore) << "Failed to get QImage when setting default texture to" << path;
            return;
        }

        m_pDefaultTexture->setData(image.mirrored());
        m_pDefaultTexture->create();
    }
}
