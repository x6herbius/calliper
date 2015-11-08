#ifndef COPENGLRENDERER_H
#define COPENGLRENDERER_H

#include <QObject>
#include <QHash>
#include <QOpenGLShader>

class CSceneObject;
class QOpenGLFunctions_3_2_Core;
class CBasicCamera;
class CResourceManager;

class COpenGLRenderer : public QObject
{
    Q_OBJECT
public:
    //////////////////////////////////////////////////////
    /// Static Stuff
    //////////////////////////////////////////////////////

    // Formats for vertex buffers.
    // Attributes are interleaved in the order specified.
    enum InterleavingFormat
    {
        FormatPosition = 0,
        FormatPositionUV,
    };

    // Attribute types.
    // The value within this enum corresponds to the expected attribute location used in the shader.
    // When writing new shaders, bind attribute locations that match these values.
    enum Attribute
    {
        Position     = 0,
        UV,         // 1
    };

    // Shader types.
    enum ShaderType
    {
        SolidColor = 0,
    };

    // These are all IN BYTES.
    // Offset from the beginning of the buffer for the first instance of the given attribute.
    static int attributeOffset(InterleavingFormat format, Attribute att);

    // How large a given attribute is.
    static int attributeSize(Attribute att);

    // How large an entire collection of attributes is.
    // This is the same as the stride required for the format.
    static int interleavingFormatSize(InterleavingFormat format);

    //////////////////////////////////////////////////////

    explicit COpenGLRenderer(CResourceManager* resourceManager, QObject *parent = 0);
    ~COpenGLRenderer();

    // Assumes context is current.
    void render(QOpenGLFunctions_3_2_Core* f, const CSceneObject* root, const CBasicCamera* camera);

signals:

public slots:

private:
    CResourceManager*   m_pResourceManager;
};

#endif // COPENGLRENDERER_H