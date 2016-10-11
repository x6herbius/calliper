#include "scenecamera.h"

namespace NS_MODEL
{
    SceneCamera::SceneCamera(Scene* parentScene, SceneObject* parentObject)
        : SceneObject(parentScene, parentObject)
    {
        commonInit();
    }

    SceneCamera::SceneCamera(const SceneCamera *cloneFrom)
        : SceneObject(cloneFrom)
    {
        commonInit();
    }

    SceneCamera::~SceneCamera()
    {

    }

    void SceneCamera::commonInit()
    {
        // Set us not to be scalable.
        updateScalableState(scalable());
    }

    bool SceneCamera::scalable() const
    {
        return false;
    }
}
