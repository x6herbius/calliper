#ifndef ORIGINMARKER_H
#define ORIGINMARKER_H

#include "model_global.h"
#include "sceneobject.h"

namespace NS_MODEL
{
    class MODELSHARED_EXPORT OriginMarker : public SceneObject
    {
        Q_OBJECT
        friend class BaseScene;
    public:
        virtual ObjectFlags objectFlags() const;
        virtual BoundingBox computeLocalBounds() const;

    protected:
        explicit OriginMarker(BaseScene* scene, SceneObject* parent);
        OriginMarker(const OriginMarker &cloneFrom);
        virtual ~OriginMarker();

    private:
        void constructGeometry();
    };
}

#endif // ORIGINMARKER_H