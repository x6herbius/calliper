#ifndef RENDERMODEL_H
#define RENDERMODEL_H

#include "renderer_global.h"

#include <QHash>
#include <QList>
#include <QMap>
#include <QLoggingCategory>

#include "renderer/rendermodel/1-passlevel/rendermodelpass.h"
#include "renderer/rendermodel/1-passlevel/rendermodelpasskey.h"
#include "renderer/shaders/globalshaderuniforms.h"
#include "renderer/opengl/openglvertexarrayobject.h"
#include "renderer/rendermodel/rendererinputobjectparams.h"
#include "renderer/rendermodel/rendererdrawparams.h"
#include "renderer/functors/renderfunctorgroup.h"

namespace Renderer
{
    Q_DECLARE_LOGGING_CATEGORY(lcRenderModelVerbose)

    class RENDERERSHARED_EXPORT RenderModel
    {
    public:
        RenderModel();
        ~RenderModel();

        IShaderRetrievalFunctor* shaderFunctor() const;
        void setShaderFunctor(IShaderRetrievalFunctor *functor);
        ITextureRetrievalFunctor* textureFunctor() const;
        void setTextureFunctor(ITextureRetrievalFunctor *functor);
        IMaterialRetrievalFunctor* materialFunctor() const;
        void setMaterialFunctor(IMaterialRetrievalFunctor* functor);
        void setRenderFunctors(const RenderFunctorGroup& renderFunctors);

        void updateObject(const RendererInputObjectParams &object);
        void removeObject(quint32 objectId);

        void draw(const RendererDrawParams &params);

        void setObjectFlags(quint32 objectId, quint32 flags);
        void clearObjectFlags(quint32 objectId, quint32 flags);
        quint32 getObjectFlags(quint32 objectId) const;

    private:
        typedef QSharedPointer<RenderModelPass> RenderModelPassPointer;
        typedef QSharedPointer<QList<RenderModelKey> > RenderModelKeyListPointer;

        RenderModelPassPointer createRenderPass(const RenderModelPassKey &key);
        RenderModelPassPointer getRenderPass(const RenderModelPassKey &key) const;
        void removeRenderPass(const RenderModelPassKey &key);
        bool containsRenderPass(const RenderModelPassKey &key);
        void clearRenderPasses();
        void uploadGlobalShaderUniforms();
        void updateGlobalShaderUniforms();
        void setObjectHidden(quint32 objectId, bool hidden);

        MatrixBatch::MatrixBatchItemPointer createOrFetchMatrixBatchItem(const RenderModelKey &key,
                                                RenderModelPass::RenderModelBatchGroupPointer* batchGroup = Q_NULLPTR);
        void cleanMatrixBatchItem(const RenderModelKey &key);
        bool getModelItems(const RenderModelKey &key,
                           RenderModelPassPointer &pass,
                           RenderModelPass::RenderModelBatchGroupPointer &batchGroup,
                           RenderModelBatchGroup::MatrixBatchPointer &matrixBatch,
                           MatrixBatch::MatrixBatchItemPointer &batchItem) const;

        RenderFunctorGroup m_RenderFunctors;
        RendererDrawParams m_DrawParams;

        GlobalShaderUniforms m_GlobalShaderUniforms;
        OpenGLVertexArrayObject m_VAO;

        QMap<RenderModelPassKey, RenderModelPassPointer>   m_RenderPasses;
        QHash<quint32, RenderModelKeyListPointer> m_StoredObjects;
        QHash<quint32, quint32> m_ObjectFlags;
    };
}

#endif // RENDERMODEL_H
