#ifndef OPENGLUNIFORMBUFFER_H
#define OPENGLUNIFORMBUFFER_H

#include "renderer_global.h"
#include <QOpenGLBuffer>

namespace Renderer
{
    class OpenGLUniformBuffer
    {
    public:
        OpenGLUniformBuffer();
        OpenGLUniformBuffer(QOpenGLBuffer::UsagePattern pattern);
        OpenGLUniformBuffer(const OpenGLUniformBuffer &other);
        ~OpenGLUniformBuffer();

        inline bool operator ==(const OpenGLUniformBuffer &other)
        {
            return m_iHandle == other.m_iHandle;
        }

        GLuint bufferId() const;

        void bind();
        void bindToIndex(int bindingPoint);
        void release();
        bool create();
        void destroy();
        bool isCreated() const;
        int size() const;

        void allocate(const void* data, int count);
        void allocate(int count);
        void write(int offset, const void *data, int count);
        void read(int offset, void *data, int count);
        void* map(QOpenGLBuffer::Access access);
        void unmap();

        // This should not be called after allocate() or write().
        QOpenGLBuffer::UsagePattern usagePattern() const;
        void setUsagePattern(QOpenGLBuffer::UsagePattern pattern);

    private:
        GLuint  m_iHandle;
        bool    m_bCreated;
        QOpenGLBuffer::UsagePattern m_iUsagePattern;
    };
}

#endif // OPENGLUNIFORMBUFFER_H
