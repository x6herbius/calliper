#ifndef BASESHADERPALETTE_H
#define BASESHADERPALETTE_H

#include "renderer_global.h"
#include <QHash>
#include "shaderdefs.h"

namespace Renderer
{
    class RENDERERSHARED_EXPORT BaseShaderPalette
    {
    public:
        BaseShaderPalette();
        virtual ~BaseShaderPalette();

        // This gives us the ID of the shader that corresponds to this technique
        // within the current rendering mode.
        virtual quint16 shader(ShaderDefs::ShaderTechnique technique) const = 0;
    };
}

#endif // BASESHADERPALETTE_H
