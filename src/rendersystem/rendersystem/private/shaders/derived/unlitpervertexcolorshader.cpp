#include "unlitpervertexcolorshader.h"

UnlitPerVertexColorShader::UnlitPerVertexColorShader(QObject* parent)
    : OpenGLShaderProgram(UnlitPerVertexColorShader::staticMetaObject.className(), parent)
{
}

UnlitPerVertexColorShader::~UnlitPerVertexColorShader()
{
}

void UnlitPerVertexColorShader::construct()
{
    addShaderFileWithCommonHeaders(QOpenGLShader::Vertex, ":/shaders/unlitpervertexcolorshader.vert");
    addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/unlitpervertexcolorshader.frag");
    link();
}

bool UnlitPerVertexColorShader::hasLocalUniformBlockBinding() const
{
    return true;
}

VertexFormat UnlitPerVertexColorShader::vertexFormat() const
{
    return VertexFormat(4, 0, 4, 0);
}

int UnlitPerVertexColorShader::maxBatchedItems() const
{
    return 8;
}