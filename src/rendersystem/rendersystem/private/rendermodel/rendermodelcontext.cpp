#include "rendermodelcontext.h"

RenderModelContext::RenderModelContext()
    : m_nRenderMode(RenderSystem::ShaderDefs::UnknownRenderMode)
{

}

RenderSystem::ShaderDefs::RenderMode RenderModelContext::renderMode() const
{
    return m_nRenderMode;
}

void RenderModelContext::setRenderMode(RenderSystem::ShaderDefs::RenderMode renderMode)
{
    m_nRenderMode = renderMode;
}
