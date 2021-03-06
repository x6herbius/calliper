#ifndef SPATIALCONFIGURATIONCHANGE_H
#define SPATIALCONFIGURATIONCHANGE_H

#include "model_global.h"
#include <QEvent>
#include <QVector3D>
#include "model/math/eulerangle.h"

namespace Model
{
    class SpatialConfigurationChange : public QEvent
    {
    public:
        enum ChangeType
        {
            PositionChange,
            RotationChange,
            ScaleChange,
        };

        SpatialConfigurationChange(ChangeType changeType);

        ChangeType changeType() const;

    private:
        ChangeType  m_iChangeType;
    };
}

#endif // SPATIALCONFIGURATIONCHANGE_H
