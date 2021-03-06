#ifndef RENDERMODELPASSKEY_H
#define RENDERMODELPASSKEY_H

#include "renderer_global.h"
#include <QHash>

namespace Renderer
{
    class RENDERERSHARED_EXPORT RenderModelPassKey
    {
    public:
        explicit RenderModelPassKey(int index);

        int passIndex() const;

        inline bool operator <(const RenderModelPassKey &other) const
        {
            return m_iPassIndex < other.m_iPassIndex;
        }

        inline bool operator ==(const RenderModelPassKey &other) const
        {
            return m_iPassIndex == other.m_iPassIndex;
        }

        inline bool operator !=(const RenderModelPassKey &other) const
        {
            return !(*this == other);
        }

    private:
        int m_iPassIndex;
    };
}

RENDERERSHARED_EXPORT uint qHash(const Renderer::RenderModelPassKey &key, uint seed = 0);

#endif // RENDERMODELPASSKEY_H
