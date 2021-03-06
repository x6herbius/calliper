#include "hashfunctions.h"
#include <QByteArray>
#include <QDataStream>

uint qHash(const QMatrix4x4 &mat, uint seed)
{
    QByteArray arr;

    {
        QDataStream stream(&arr, QIODevice::WriteOnly);
        stream.writeBytes(reinterpret_cast<const char*>(mat.constData()), 16 * sizeof(float));
    }

    return qHash(arr, seed);
}

uint qHash(const QVector3D &vec, uint seed)
{
    QByteArray arr;

    {
        QDataStream stream(&arr, QIODevice::WriteOnly);
        stream << vec.x() << vec.y() << vec.z();
    }

    return qHash(arr, seed);
}
