#ifndef ERRORSHADER_H
#define ERRORSHADER_H

#include "rendersystem/private/shaders/base/openglshaderprogram.h"
#include "rendersystem/private/shaders/common/batchedmodelmatrixuniformblock.h"

class ErrorShader : public OpenGLShaderProgram
{
    Q_OBJECT
public:
    typedef BatchedModelMatrixUniformBlock<8> UniformBlock;

    ErrorShader(QObject* parent = 0);
    virtual ~ErrorShader();

    static PrivateShaderDefs::ShaderId shaderId();

    virtual void construct() override;
    virtual bool hasLocalUniformBlockBinding() const override;
    virtual VertexFormat vertexFormat() const override;
    virtual int maxBatchedItems() const override;
};

#endif // ERRORSHADER_H
