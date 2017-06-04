#ifndef IFRAMEBUFFERSTORE_H
#define IFRAMEBUFFERSTORE_H

#include "rendersystem_global.h"

#include <QSize>
#include <QOpenGLFunctions>

#include "rendersystem/interface-classes/definitions/framebufferdefs.h"

namespace RenderSystem
{
    class IFrameBufferStore
    {
    public:
        virtual ~IFrameBufferStore() {}

        // Creates a new frame buffer with the given size.
        virtual FrameBufferDefs::FrameBufferId createFrameBuffer(const QSize& size) = 0;

        // Destroys the given frame buffer.
        virtual void removeFrameBuffer(const FrameBufferDefs::FrameBufferId id) = 0;

        // Returns whether a frame buffer exists for the given ID.
        virtual bool frameBufferExists(const FrameBufferDefs::FrameBufferId id) const = 0;

        // Returns the size of the frame buffer.
        // When a frame buffer is rendered into, the size will be set automatically.
        virtual QSize frameBufferSize(const FrameBufferDefs::FrameBufferId id) const = 0;

        // Returns the OpenGL texture ID for this frame buffer.
        // If the provided frame buffer doesnt' exist, 0 will be returned
        // (and should be considered invalid).
        virtual GLuint frameBufferTextureId(const FrameBufferDefs::FrameBufferId id) const = 0;
    };
}

#endif // IFRAMEBUFFERSTORE_H