#include "openglerrors.h"

namespace NS_RENDERER
{
    OpenGLErrors::OpenGLErrors()
    {
    }

    QString OpenGLErrors::errorString(GLenum errorCode)
    {
        int enumIndex = staticMetaObject.indexOfEnumerator("OpenGLErrors");
        QMetaEnum metaEnum = staticMetaObject.enumerator(enumIndex);
        return QString(metaEnum.valueToKey(errorCode));
    }
}