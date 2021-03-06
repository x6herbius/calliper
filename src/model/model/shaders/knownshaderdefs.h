#ifndef KNOWNSHADERDEFS_H
#define KNOWNSHADERDEFS_H

#include "model_global.h"
#include <QObject>

namespace Model
{
    class KnownShaderDefs
    {
        Q_GADGET
        KnownShaderDefs() = delete;
    public:
        enum KnownShaderId
        {
            UnknownShaderId = 0,

            SimpleLitShaderId,
            UnlitPerVertexColourShaderId,

            TOTAL_SHADERS
        };
        Q_ENUM(KnownShaderId)
    };
}

#endif // KNOWNSHADERDEFS_H
