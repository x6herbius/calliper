#ifndef OPENGLERRORS_H
#define OPENGLERRORS_H

#include "calliperutil_global.h"

#include <QObject>
#include <QtOpenGL>
#include <QOpenGLContext>
#include <QtDebug>

class CALLIPERUTILSHARED_EXPORT OpenGLErrors
{
    Q_GADGET
    OpenGLErrors()  = delete;
public:
    enum OpenGLError : GLenum
    {
        NoError                     = GL_NO_ERROR,
        InvalidEnum                 = GL_INVALID_ENUM,
        InvalidValue                = GL_INVALID_VALUE,
        InvalidOperation            = GL_INVALID_OPERATION,
        InvalidFrameBufferOperation = GL_INVALID_FRAMEBUFFER_OPERATION,
        OutOfMemory                 = GL_OUT_OF_MEMORY,
        StackUnderflow              = GL_STACK_UNDERFLOW,
        StackOverflow               = GL_STACK_OVERFLOW,
    };
    Q_ENUM(OpenGLError)

    static QString errorString(GLenum errorCode);
    static QString debugOpenGLCapabilities();

    static inline bool glTry(std::function<void(void)> function,
                      const char* funcName,
                      const char* file,
                      int line,
                      const char* parentFuncName)
    {
        function();

#ifdef QT_DEBUG
        QStringList errorList = getErrorList();
        if ( !errorList.isEmpty() )
        {
            qFatal("%s", QString("OpenGL Error: in function %1 (%2:%3), '%4' returned the following errors: %5")
                   .arg(parentFuncName)
                   .arg(file)
                   .arg(line)
                   .arg(funcName)
                   .arg(errorList.join(", "))
                   .toUtf8().constData()
                  );

            // For some reason qFatal sometimes doesn't kill us on Windows...?
            Q_ASSERT(false);

            return false;
        }

        return true;
#else
        return hasError();
#endif
    }

    static inline QStringList getErrorList()
    {
        QStringList errorList;

        for (GLenum ret = QOpenGLContext::currentContext()->functions()->glGetError(); ret != GL_NO_ERROR;
                ret = QOpenGLContext::currentContext()->functions()->glGetError())
        {
            errorList.append(OpenGLErrors::errorString(ret));
        }

        return errorList;
    }

    static inline bool hasError()
    {
        return QOpenGLContext::currentContext()->functions()->glGetError() != GL_NO_ERROR;
    }
};

// Compiles out of release builds.
#ifdef QT_DEBUG
#define GLTRY(_func) ::OpenGLErrors::glTry([&]{_func;}, #_func, __FILE__, __LINE__, Q_FUNC_INFO)
#else
#define GLTRY(_func) _func
#endif

// Remains in release builds and just returns a boolean success code.
#define GLTRY_RET(_func) ::OpenGLErrors::glTry([&]{_func;}, #_func, __FILE__, __LINE__, Q_FUNC_INFO)

#endif // OPENGLERRORS_H
