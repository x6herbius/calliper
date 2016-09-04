#include "tempshader.h"

static const char *vertexShaderSource =
        "#version 410 core\n"
        "layout (location=0) in vec4 vPosition;\n"
        "layout (location=2) in vec4 vColour;\n"
        "layout (location=3) in vec2 vTexCoord;\n"
        "layout (std140) uniform LocalUniformBlock\n"
        "{\n"
        "   mat4 modelToWorldMatrices[8];\n"
        "};\n"
        "out vec4 fColour;\n"
        "out vec2 fTexCoord;\n"
        "void main()\n"
        "{\n"
        "   uint id = uint(vPosition.z);\n"
        "   gl_Position = modelToWorldMatrices[id] * vec4(vPosition.xy, 0, 1);\n"
        "   fColour = vColour;\n"
        "   fTexCoord = vTexCoord;\n"
        "}\n"
    ;

static const char *fragmentShaderSource =
        "#version 410 core\n"
        "in vec4 fColour;\n"
        "in vec2 fTexCoord;\n"
        "layout(location = 0) out vec4 color;\n"
        "uniform sampler2D tex;\n"
        "void main()\n"
        "{\n"
        "   color = texture(tex, fTexCoord) /* * fColour*/;\n"
        "}\n"
    ;

using namespace NS_RENDERER;

TempShader::TempShader(QObject* parent)
    : OpenGLShaderProgram(QString(staticMetaObject.className()).remove(NS_STR_RENDERER "::"), parent)
{

}

TempShader::~TempShader()
{

}

void TempShader::construct()
{
    addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    link();
}