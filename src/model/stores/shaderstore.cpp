#include "shaderstore.h"

namespace NS_MODEL
{
    ShaderStore::ShaderStore()
        : m_iNextShaderId(1)
    {

    }

    ShaderStore::~ShaderStore()
    {
        using namespace NS_RENDERER;

        foreach ( OpenGLShaderProgram* shaderProgram, m_ShaderTable.values() )
        {
            delete shaderProgram;
        }

        m_ShaderTable.clear();
    }

    quint16 ShaderStore::acquireNextShaderId()
    {
        // At 1 count per second, this should take 136 years to overflow.
        Q_ASSERT_X(m_iNextShaderId + 1 != 0, Q_FUNC_INFO, "Shader ID counter overflow!");
        return m_iNextShaderId++;
    }

    NS_RENDERER::OpenGLShaderProgram* ShaderStore::getShaderProgram(quint16 shaderId) const
    {
        return m_ShaderTable.value(shaderId, NULL);
    }

    NS_RENDERER::OpenGLShaderProgram* ShaderStore::operator ()(quint16 shaderId) const
    {
        return getShaderProgram(shaderId);
    }

    quint16 ShaderStore::getShaderId(const QString shaderName) const
    {
        using namespace NS_RENDERER;

        foreach ( OpenGLShaderProgram* shaderProgram, m_ShaderTable.values() )
        {
            if ( shaderProgram->objectName() == shaderName )
                return shaderProgram->shaderStoreId();
        }

        return 0;
    }

    NS_RENDERER::OpenGLShaderProgram* ShaderStore::getShaderProgram(const QString &shaderName) const
    {
        return getShaderProgram(getShaderId(shaderName));
    }
}
