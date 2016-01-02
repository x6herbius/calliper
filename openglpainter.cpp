#include "shaderstack.h"
#include "shaderprogram.h"

ShaderStack::ShaderStack(ShaderProgram* initial, bool autoUpdate)
{
    m_bAutoUpdate = autoUpdate;
    m_pCamera = NULL;
    m_Shaders.push(initial);
    m_Shaders.top()->apply();

    m_ModelToWorld.push(QMatrix4x4());
    m_WorldToCamera.push(QMatrix4x4());
    m_CoordinateTransform.push(QMatrix4x4());
    m_CameraProjection.push(QMatrix4x4());
    m_FogColor.push(QColor::fromRgb(0xffffffff));
    m_FogBegin.push(0);
    m_FogEnd.push(0);
    m_DirectionalLight.push(QVector3D(1,0,0));
    m_GlobalColor.push(QColor::fromRgb(0xffffffff));

    if ( m_bAutoUpdate )
        applyAll();
}

ShaderStack::~ShaderStack()
{
    ShaderProgram* p = m_Shaders.top();
    if ( p )
        p->release();
}

const Camera* ShaderStack::camera() const
{
    return m_pCamera;
}

void ShaderStack::setCamera(const Camera *camera)
{
    m_pCamera = camera;
}

bool ShaderStack::autoUpdate() const
{
    return m_bAutoUpdate;
}

void ShaderStack::setAutoUpdate(bool enabled)
{
    m_bAutoUpdate = enabled;
}

bool ShaderStack::inInitialState() const
{
    return m_Shaders.count() == 1 &&
            m_ModelToWorld.count() == 1 &&
            m_WorldToCamera.count() == 1 &&
            m_CoordinateTransform.count() == 1 &&
            m_CameraProjection.count() == 1 &&
            m_FogColor.count() == 1 &&
            m_FogBegin.count() == 1 &&
            m_FogEnd.count() == 1 &&
            m_DirectionalLight.count() == 1 &&
            m_GlobalColor.count() == 1;
}

// Should be called whenever a new shader is applied.
void ShaderStack::applyAll()
{
    ShaderProgram* p = m_Shaders.top();
    p->setUniformMatrix4(ShaderProgram::ModelToWorldMatrix, m_ModelToWorld.top());
    p->setUniformMatrix4(ShaderProgram::WorldToCameraMatrix, m_WorldToCamera.top());
    p->setUniformMatrix4(ShaderProgram::CoordinateTransformMatrix, m_CoordinateTransform.top());
    p->setUniformMatrix4(ShaderProgram::CameraProjectionMatrix, m_CameraProjection.top());
    p->setUniformColor4(ShaderProgram::FogColorUniform, m_FogColor.top());
    p->setUniformFloat(ShaderProgram::FogBeginUniform, m_FogBegin.top());
    p->setUniformFloat(ShaderProgram::FogEndUniform, m_FogEnd.top());
    p->setUniformVector3(ShaderProgram::DirectionalLightUniform, m_DirectionalLight.top());
    p->setUniformColor4(ShaderProgram::ColorUniform, m_GlobalColor.top());
}

void ShaderStack::shaderPush(ShaderProgram *program)
{
    m_Shaders.top()->release();
    m_Shaders.push(program);
    m_Shaders.top()->apply();
    if ( m_bAutoUpdate )
        applyAll();
}

void ShaderStack::shaderPop()
{
    m_Shaders.top()->release();
    m_Shaders.pop();
    Q_ASSERT(m_Shaders.count() >= 1);
    m_Shaders.top()->apply();
    if ( m_bAutoUpdate )
        applyAll();
}

ShaderProgram* ShaderStack::shaderTop() const
{
    return m_Shaders.top();
}

int ShaderStack::shaderCount() const
{
    return m_Shaders.count();
}

void ShaderStack::modelToWorldPostMultiply(const QMatrix4x4 &mat)
{
    postMultiplyTop(m_ModelToWorld, ShaderProgram::ModelToWorldMatrix, mat);
}

void ShaderStack::modelToWorldPush()
{
    m_ModelToWorld.push(m_ModelToWorld.top());
}

void ShaderStack::modelToWorldPop()
{
    pop(m_ModelToWorld, ShaderProgram::ModelToWorldMatrix);
}

void ShaderStack::modelToWorldPreMultiply(const QMatrix4x4 &mat)
{
    preMultiplyTop(m_ModelToWorld, ShaderProgram::ModelToWorldMatrix, mat);
}

const QMatrix4x4& ShaderStack::modelToWorldTop() const
{
    return m_ModelToWorld.top();
}

int ShaderStack::modelToWorldCount() const
{
    return m_ModelToWorld.count();
}

void ShaderStack::preMultiplyTop(QStack<QMatrix4x4> &stack, ShaderProgram::Attribute att, const QMatrix4x4 &mat)
{
    stack.top() = mat * stack.top();
    if ( m_bAutoUpdate )
        m_Shaders.top()->setUniformMatrix4(att, stack.top());
}

void ShaderStack::postMultiplyTop(QStack<QMatrix4x4> &stack, ShaderProgram::Attribute att, const QMatrix4x4 &mat)
{
    stack.top() = stack.top() * mat;
    if ( m_bAutoUpdate )
        m_Shaders.top()->setUniformMatrix4(att, stack.top());
}

void ShaderStack::pop(QStack<QMatrix4x4> &stack, ShaderProgram::Attribute att)
{
    stack.pop();
    Q_ASSERT(stack.count() >= 1);
    if ( m_bAutoUpdate )
        m_Shaders.top()->setUniformMatrix4(att, stack.top());
}

void ShaderStack::worldToCameraPreMultiply(const QMatrix4x4 &mat)
{
    preMultiplyTop(m_WorldToCamera, ShaderProgram::WorldToCameraMatrix, mat);
}

void ShaderStack::worldToCameraPostMultiply(const QMatrix4x4 &mat)
{
    postMultiplyTop(m_WorldToCamera, ShaderProgram::WorldToCameraMatrix, mat);
}

void ShaderStack::worldToCameraPush()
{
    m_WorldToCamera.push(m_WorldToCamera.top());
}

void ShaderStack::worldToCameraPop()
{
    pop(m_WorldToCamera, ShaderProgram::WorldToCameraMatrix);
}

const QMatrix4x4& ShaderStack::worldToCameraTop() const
{
    return m_WorldToCamera.top();
}

int ShaderStack::worldToCameraCount() const
{
    return m_WorldToCamera.count();
}

void ShaderStack::coordinateTransformPreMultiply(const QMatrix4x4 &mat)
{
    preMultiplyTop(m_CoordinateTransform, ShaderProgram::CoordinateTransformMatrix, mat);
}

void ShaderStack::coordinateTransformPostMultiply(const QMatrix4x4 &mat)
{
    postMultiplyTop(m_CoordinateTransform, ShaderProgram::CoordinateTransformMatrix, mat);
}

void ShaderStack::coordinateTransformPush()
{
    m_CoordinateTransform.push(m_CoordinateTransform.top());
}

void ShaderStack::coordinateTransformPop()
{
    pop(m_CoordinateTransform, ShaderProgram::CoordinateTransformMatrix);
}

const QMatrix4x4& ShaderStack::coordinateTransformTop() const
{
    return m_CoordinateTransform.top();
}

int ShaderStack::coordinateTransformCount() const
{
    return m_CoordinateTransform.count();
}

void ShaderStack::cameraProjectionPreMultiply(const QMatrix4x4 &mat)
{
    preMultiplyTop(m_CameraProjection, ShaderProgram::CameraProjectionMatrix, mat);
}

void ShaderStack::cameraProjectionPostMultiply(const QMatrix4x4 &mat)
{
    postMultiplyTop(m_CameraProjection, ShaderProgram::CameraProjectionMatrix, mat);
}

void ShaderStack::cameraProjectionPush()
{
    m_CameraProjection.push(m_CameraProjection.top());
}

void ShaderStack::cameraProjectionPop()
{
    pop(m_CameraProjection, ShaderProgram::CameraProjectionMatrix);
}

const QMatrix4x4& ShaderStack::cameraProjectionTop() const
{
    return m_CameraProjection.top();
}

int ShaderStack::cameraProjectionCount() const
{
    return m_CameraProjection.count();
}

void ShaderStack::pop(QStack<QColor> &stack, ShaderProgram::Attribute att)
{
    stack.pop();
    Q_ASSERT(stack.count() >= 1);
    if ( m_bAutoUpdate )
        m_Shaders.top()->setUniformColor4(att, stack.top());
}

void ShaderStack::setTop(QStack<QColor> &stack, ShaderProgram::Attribute att, const QColor &col)
{
    if ( col == stack.top() ) return;

    stack.top() = col;
    if ( m_bAutoUpdate )
        m_Shaders.top()->setUniformColor4(att, stack.top());
}

void ShaderStack::fogColorSetTop(const QColor &col)
{
    setTop(m_FogColor, ShaderProgram::FogColorUniform, col);
}

void ShaderStack::fogColorPush()
{
    m_FogColor.push(m_FogColor.top());
}

void ShaderStack::fogColorPop()
{
    pop(m_FogColor, ShaderProgram::FogColorUniform);
}
const QColor& ShaderStack::fogColorTop() const
{
    return m_FogColor.top();
}

int ShaderStack::fogColorCount() const
{
    return m_FogColor.count();
}

void ShaderStack::setTop(QStack<float> &stack, ShaderProgram::Attribute att, float value)
{
    if ( value == stack.top() ) return;

    stack.top() = value;
    if ( m_bAutoUpdate )
        m_Shaders.top()->setUniformFloat(att, stack.top());
}

void ShaderStack::pop(QStack<float> &stack, ShaderProgram::Attribute att)
{
    stack.pop();
    Q_ASSERT(stack.count() >= 1);
    if ( m_bAutoUpdate )
        m_Shaders.top()->setUniformFloat(att, stack.top());
}

void ShaderStack::fogBeginSetTop(float val)
{
    setTop(m_FogBegin, ShaderProgram::FogBeginUniform, val);
}

void ShaderStack::fogBeginPush()
{
    m_FogBegin.push(m_FogBegin.top());
}

void ShaderStack::fogBeginPop()
{
    pop(m_FogBegin, ShaderProgram::FogBeginUniform);
}

const float& ShaderStack::fogBeginTop() const
{
    return m_FogBegin.top();
}

int ShaderStack::fogBeginCount() const
{
    return m_FogBegin.count();
}

void ShaderStack::fogEndSetTop(float val)
{
    setTop(m_FogEnd, ShaderProgram::FogEndUniform, val);
}

void ShaderStack::fogEndPush()
{
    m_FogEnd.push(m_FogEnd.top());
}

void ShaderStack::fogEndPop()
{
    pop(m_FogEnd, ShaderProgram::FogEndUniform);
}

const float& ShaderStack::fogEndTop() const
{
    return m_FogEnd.top();
}

int ShaderStack::fogEndCount() const
{
    return m_FogEnd.count();
}

void ShaderStack::setTop(QStack<QVector3D> &stack, ShaderProgram::Attribute att, const QVector3D &value)
{
    if ( value == stack.top() ) return;

    stack.top() = value;
    if ( m_bAutoUpdate )
        m_Shaders.top()->setUniformVector3(att, stack.top());
}

void ShaderStack::pop(QStack<QVector3D> &stack, ShaderProgram::Attribute att)
{
    stack.pop();
    Q_ASSERT(stack.count() >= 1);
    if ( m_bAutoUpdate )
        m_Shaders.top()->setUniformVector3(att, stack.top());
}

void ShaderStack::directionalLightSetTop(const QVector3D &val)
{
    setTop(m_DirectionalLight, ShaderProgram::DirectionalLightUniform, val);
}

void ShaderStack::directionalLightPush()
{
    m_DirectionalLight.push(m_DirectionalLight.top());
}

void ShaderStack::directionalLightPop()
{
    pop(m_DirectionalLight, ShaderProgram::DirectionalLightUniform);
}

const QVector3D& ShaderStack::directionalLightTop() const
{
    return m_DirectionalLight.top();
}

int ShaderStack::directionalLightCount() const
{
    return m_DirectionalLight.count();
}

void ShaderStack::globalColorSetTop(const QColor &col)
{
    setTop(m_GlobalColor, ShaderProgram::ColorUniform, col);
}

void ShaderStack::globalColorPush()
{
    m_GlobalColor.push(m_GlobalColor.top());
}

void ShaderStack::globalColorPop()
{
    pop(m_GlobalColor, ShaderProgram::ColorUniform);
}

const QColor& ShaderStack::globalColorTop() const
{
    return m_GlobalColor.top();
}

int ShaderStack::globalColorCount() const
{
    return m_GlobalColor.count();
}

void ShaderStack::setToIdentity(QStack<QMatrix4x4> &stack, ShaderProgram::Attribute att)
{
    stack.top().setToIdentity();
    if ( m_bAutoUpdate )
        m_Shaders.top()->setUniformMatrix4(att, stack.top());
}

void ShaderStack::modelToWorldSetToIdentity()
{
    if ( m_ModelToWorld.top().isIdentity() ) return;
    setToIdentity(m_ModelToWorld, ShaderProgram::ModelToWorldMatrix);
}

void ShaderStack::worldToCameraSetToIdentity()
{
    if ( m_WorldToCamera.top().isIdentity() ) return;
    setToIdentity(m_WorldToCamera, ShaderProgram::WorldToCameraMatrix);
}

void ShaderStack::coordinateTransformSetToIdentity()
{
    if ( m_CoordinateTransform.top().isIdentity() ) return;
    setToIdentity(m_CoordinateTransform, ShaderProgram::CoordinateTransformMatrix);
}

void ShaderStack::cameraProjectionSetToIdentity()
{
    if ( m_CameraProjection.top().isIdentity() ) return;
    setToIdentity(m_CameraProjection, ShaderProgram::CameraProjectionMatrix);
}