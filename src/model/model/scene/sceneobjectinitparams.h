#ifndef SCENEOBJECTINITPARAMS_H
#define SCENEOBJECTINITPARAMS_H

#include "model_global.h"

#include "rendersystem/interface-classes/definitions/rendermodeldefs.h"

namespace Model
{
    class Scene;

    class MODELSHARED_EXPORT SceneObjectInitParams
    {
    public:
        SceneObjectInitParams(Scene* parentScene, RenderSystem::RenderModelDefs::ObjectId objectId);

        Scene* m_pScene;
        RenderSystem::RenderModelDefs::ObjectId m_iObjectId;
    };
}

#endif // SCENEOBJECTINITPARAMS_H
