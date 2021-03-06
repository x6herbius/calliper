#include "rendermodel.h"
#include <QtDebug>
#include "renderer/opengl/openglerrors.h"
#include "renderer/rendermodel/rendererobjectflags.h"
#include "renderer/opengl/scopedcurrentcontext.h"

namespace
{
    inline bool flagIsSet(quint32 value, quint32 flag)
    {
        return (value & flag) == flag;
    }

    inline bool flagWasSet(quint32 before, quint32 after, quint32 flag)
    {
        return !flagIsSet(before, flag) && flagIsSet(after, flag);
    }

    inline bool flagWasUnset(quint32 before, quint32 after, quint32 flag)
    {
        return flagWasSet(after, before, flag);
    }
}

namespace Renderer
{
    Q_LOGGING_CATEGORY(lcRenderModelVerbose, "Renderer.RenderModel.Verbose")

    RenderModel::RenderModel()
        : m_RenderFunctors(),
          m_DrawParams(),
          m_GlobalShaderUniforms(QOpenGLBuffer::DynamicDraw)
    {
        ScopedCurrentContext scopedContext;
        Q_UNUSED(scopedContext);

        m_VAO.create();
        m_GlobalShaderUniforms.create();
        updateGlobalShaderUniforms();
    }

    RenderModel::~RenderModel()
    {
        ScopedCurrentContext scopedContext;
        Q_UNUSED(scopedContext);

        clearRenderPasses();
        m_GlobalShaderUniforms.destroy();
        m_VAO.destroy();
    }

    RenderModel::RenderModelPassPointer RenderModel::createRenderPass(const RenderModelPassKey &key)
    {
        RenderModelPassPointer pass = RenderModelPassPointer::create(m_RenderFunctors);
        m_RenderPasses.insert(key, pass);
        return pass;
    }

    RenderModel::RenderModelPassPointer RenderModel::getRenderPass(const RenderModelPassKey &key) const
    {
        return m_RenderPasses.value(key, RenderModelPassPointer());
    }

    void RenderModel::removeRenderPass(const RenderModelPassKey &key)
    {
        m_RenderPasses.remove(key);
    }

    bool RenderModel::containsRenderPass(const RenderModelPassKey &key)
    {
        return m_RenderPasses.contains(key);
    }

    void RenderModel::clearRenderPasses()
    {
        m_RenderPasses.clear();
    }

    IShaderRetrievalFunctor* RenderModel::shaderFunctor() const
    {
        return m_RenderFunctors.shaderFunctor;
    }

    void RenderModel::setShaderFunctor(IShaderRetrievalFunctor *functor)
    {
        m_RenderFunctors.shaderFunctor = functor;
    }

    ITextureRetrievalFunctor* RenderModel::textureFunctor() const
    {
        return m_RenderFunctors.textureFunctor;
    }

    void RenderModel::setTextureFunctor(ITextureRetrievalFunctor *functor)
    {
        m_RenderFunctors.textureFunctor = functor;
    }

    IMaterialRetrievalFunctor* RenderModel::materialFunctor() const
    {
        return m_RenderFunctors.materialFunctor;
    }

    void RenderModel::setMaterialFunctor(IMaterialRetrievalFunctor *functor)
    {
        m_RenderFunctors.materialFunctor = functor;
    }

    void RenderModel::setRenderFunctors(const RenderFunctorGroup &renderFunctors)
    {
        m_RenderFunctors = renderFunctors;
    }

    void RenderModel::updateObject(const RendererInputObjectParams &object)
    {
        // This could be optimised by checking whether any keys matched
        // the ones incoming in this update, and only clearing (not
        // removing) entries for the ones that match.
        // For now, we just remove any previous entries.
        removeObject(object.objectId());

        // Don't bother going through all this if we don't actually have any geometry.
        bool allEmpty = true;
        foreach ( GeometrySection* section, object.geometrySectionList() )
        {
            if ( !section->isEmpty() )
            {
                allEmpty = false;
                break;
            }
        }

        if ( allEmpty )
            return;

        RenderModelPassKey passKey(object.passIndex());
        bool drawable = true;
        if ( m_ObjectFlags.contains(object.objectId()) )
        {
            drawable = (m_ObjectFlags.value(object.objectId()) & HiddenObjectFlag) == HiddenObjectFlag;
        }

        RenderModelKeyListPointer list = RenderModelKeyListPointer::create();

        quint8 sectionId = 0;
        foreach ( GeometrySection* section, object.geometrySectionList() )
        {
            RenderModelKey key = RenderModelKey(
                            passKey,
                            RenderModelBatchGroupKey(
                                section->shaderId(),
                                section->materialId(),
                                section->drawMode(),
                                section->drawWidth()
                            ),
                            MatrixBatchKey(section->modelToWorldMatrix()),
                            MatrixBatchItemKey(object.objectId(), sectionId)
                        );

            RenderModelPass::RenderModelBatchGroupPointer batchGroup;
            MatrixBatch::MatrixBatchItemPointer batchItem = createOrFetchMatrixBatchItem(key, &batchGroup);
            batchItem->clear();

            section->consolidate(batchItem->m_Positions,
                                batchItem->m_Normals,
                                batchItem->m_Colors,
                                batchItem->m_TextureCoordinates,
                                batchItem->m_Indices);

            list->append(key);
            batchGroup->setMatrixBatchDrawable(key.matrixBatchKey(), drawable);

            sectionId++;
        }

        m_StoredObjects.insert(object.objectId(), list);
        m_ObjectFlags.insert(object.objectId(), NoObjectFlag);
    }

    void RenderModel::removeObject(quint32 objectId)
    {
        if ( m_StoredObjects.contains(objectId) )
        {
            RenderModelKeyListPointer keysForObject = m_StoredObjects.take(objectId);
            foreach ( const RenderModelKey &key, *keysForObject.data() )
            {
                cleanMatrixBatchItem(key);
            }
        }

        // We've already taken from stored objects.
        // Remove flags record as well.
        m_ObjectFlags.remove(objectId);
    }

    bool RenderModel::getModelItems(const RenderModelKey &key,
                                    RenderModelPassPointer &pass,
                                    RenderModelPass::RenderModelBatchGroupPointer &batchGroup,
                                    RenderModelBatchGroup::MatrixBatchPointer &matrixBatch,
                                    MatrixBatch::MatrixBatchItemPointer &batchItem) const
    {
        pass = getRenderPass(key.passKey());
        if ( pass.isNull() )
        {
            return false;
        }

        batchGroup = pass->getBatchGroup(key.batchGroupKey());
        if ( batchGroup.isNull() )
        {
            return false;
        }

        matrixBatch = batchGroup->getMatrixBatch(key.matrixBatchKey());
        if ( matrixBatch.isNull() )
        {
            return false;
        }

        batchItem = matrixBatch->getItem(key.matrixBatchItemKey());
        return !batchItem.isNull();
    }

    MatrixBatch::MatrixBatchItemPointer RenderModel::createOrFetchMatrixBatchItem(const RenderModelKey &key,
                                                     RenderModelPass::RenderModelBatchGroupPointer* batchGroupOut)
    {
        // 1: Get the pass.
        RenderModelPassPointer pass = getRenderPass(key.passKey());
        if ( pass.isNull() )
        {
            pass = createRenderPass(key.passKey());
        }

        // 2: Get the batch group.
        // At some point we'll probably want to deal with usage patterns too.
        RenderModelPass::RenderModelBatchGroupPointer batchGroup = pass->getBatchGroup(key.batchGroupKey());
        if ( batchGroup.isNull() )
        {
            batchGroup = pass->createBatchGroup(key.batchGroupKey());
        }
        if ( batchGroup )
        {
            *batchGroupOut = batchGroup;
        }

        // 3: Get the batch.
        RenderModelBatchGroup::MatrixBatchPointer matrixBatch = batchGroup->getMatrixBatch(key.matrixBatchKey());
        if ( matrixBatch.isNull() )
        {
            matrixBatch = batchGroup->createMatrixBatch(key.matrixBatchKey());
        }

        // 4: Get the batch item.
        MatrixBatch::MatrixBatchItemPointer batchItem = matrixBatch->getItem(key.matrixBatchItemKey());
        if ( batchItem.isNull() )
        {
            batchItem = matrixBatch->createItem(key.matrixBatchItemKey());
        }

        return batchItem;
    }

    void RenderModel::cleanMatrixBatchItem(const RenderModelKey &key)
    {
        RenderModelPassPointer pass;
        RenderModelPass::RenderModelBatchGroupPointer batchGroup;
        RenderModelBatchGroup::MatrixBatchPointer matrixBatch;
        MatrixBatch::MatrixBatchItemPointer batchItem;

        if ( !getModelItems(key, pass, batchGroup, matrixBatch, batchItem) )
        {
            return;
        }

        batchItem.clear();
        matrixBatch->removeItem(key.matrixBatchItemKey());

        if ( matrixBatch->itemCount() < 1 )
        {
            batchGroup->removeMatrixBatch(key.matrixBatchKey());
        }

        if ( batchGroup->matrixBatchCount() < 1 )
        {
            pass->removeBatchGroup(key.batchGroupKey());
        }
    }

    void RenderModel::draw(const RendererDrawParams &params)
    {
        if ( params != m_DrawParams )
        {
            m_DrawParams = params;
            updateGlobalShaderUniforms();
        }

        m_VAO.bind();

        uploadGlobalShaderUniforms();

        foreach ( const RenderModelPassPointer &pass, m_RenderPasses.values() )
        {
            pass->drawAllBatchGroups();
        }

        m_VAO.release();
    }

    void RenderModel::uploadGlobalShaderUniforms()
    {
        m_GlobalShaderUniforms.upload();
    }

    void RenderModel::updateGlobalShaderUniforms()
    {
        m_GlobalShaderUniforms.setWorldToCameraMatrix(m_DrawParams.worldToCameraMatrix());
        m_GlobalShaderUniforms.setProjectionMatrix(m_DrawParams.projectionMatrix());
        // TODO: Set directional light.
    }

    void RenderModel::setObjectFlags(quint32 objectId, quint32 flags)
    {
        if ( !m_ObjectFlags.contains(objectId) )
            return;

        quint32 oldObjFlags = m_ObjectFlags.value(objectId);
        quint32 newObjFlags = oldObjFlags | flags;
        m_ObjectFlags.insert(objectId, newObjFlags);

        if ( flagWasSet(oldObjFlags, newObjFlags, HiddenObjectFlag) )
        {
            setObjectHidden(objectId, true);
        }
    }

    void RenderModel::clearObjectFlags(quint32 objectId, quint32 flags)
    {
        if ( !m_ObjectFlags.contains(objectId) )
            return;

        quint32 oldObjFlags = m_ObjectFlags.value(objectId);
        quint32 newObjFlags = oldObjFlags & ~flags;
        m_ObjectFlags.insert(objectId, newObjFlags);

        if ( flagWasUnset(oldObjFlags, newObjFlags, HiddenObjectFlag) )
        {
            setObjectHidden(objectId, false);
        }
    }

    quint32 RenderModel::getObjectFlags(quint32 objectId) const
    {
        return m_ObjectFlags.value(objectId, NoObjectFlag);
    }

    void RenderModel::setObjectHidden(quint32 objectId, bool hidden)
    {
        Q_ASSERT_X(m_StoredObjects.contains(objectId), Q_FUNC_INFO, "Object does not exist!");
        RenderModelKeyListPointer list = m_StoredObjects.value(objectId);

        foreach ( const RenderModelKey &key, *(list.data()) )
        {
            RenderModelPassPointer pass = getRenderPass(key.passKey());
            if ( pass.isNull() )
                continue;

            RenderModelPass::RenderModelBatchGroupPointer batchGroup = pass->getBatchGroup(key.batchGroupKey());
            if ( batchGroup.isNull() )
                continue;

            batchGroup->setMatrixBatchDrawable(key.matrixBatchKey(), !hidden);
        }
    }
}
