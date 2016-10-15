#ifndef GENERICBRUSHFACE_H
#define GENERICBRUSHFACE_H

#include "model_global.h"
#include <QVector>
#include <QVector3D>
#include <QObject>
#include "textureplane.h"
#include "geometry/geometrybuilder.h"

namespace NS_MODEL
{
    class GenericBrush;

    class GenericBrushFace : public QObject
    {
        Q_OBJECT
    public:
        GenericBrushFace(GenericBrush* parentBrush);

        GenericBrush* parentBrush() const;

        int indexAt(int index) const;
        void appendIndex(int i);
        int indexCount() const;
        void removeIndex(int index);
        QVector<int> indexList() const;
        void clearIndices();
        QVector<QVector3D> referencedBrushVertexList() const;

        QVector3D normal() const;

        void buildFaceGeometry(NS_RENDERER::GeometryBuilder &builder);

    signals:
        void dataChanged();

    private:
        QVector<int>    m_BrushVertexIndices;
        TexturePlane*   m_pTexturePlane;
    };
}

#endif // GENERICBRUSHFACE_H
