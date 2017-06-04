#ifndef MATERIALSTORE_H
#define MATERIALSTORE_H

#include <QHash>

#include "rendersystem/interfaces/imaterialstore.h"

#include "rendersystem/interface-classes/definitions/materialdefs.h"
#include "rendersystem/interface-classes/rendermaterial/rendermaterial.h"

#include "containers/path-managing/pathmanagingobjectstore.h"

#include "calliperutil/global/globalinstancehelper.h"

class MaterialStore : public Containers::PathManagingObjectStore<RenderSystem::RenderMaterial,
                                                                 RenderSystem::MaterialDefs::MaterialId>,
                      public RenderSystem::IMaterialStore,
                      public CalliperUtil::GlobalInstanceHelper<MaterialStore>
{
public:
    typedef RenderSystem::MaterialDefs::MaterialId MaterialId;

    MaterialStore();
    virtual ~MaterialStore();

    // External
    virtual MaterialId createMaterial(const QString& path) override;
    virtual void removeMaterial(const MaterialId id) override;
    virtual QWeakPointer<RenderSystem::RenderMaterial> material(const MaterialId id) const override;
    virtual QWeakPointer<RenderSystem::RenderMaterial> material(const QString& path) const override;
    virtual bool containsMaterial(const MaterialId id) const override;
    virtual bool containsMaterial(const QString& path) const override;
    virtual MaterialId materialIdFromPath(const QString& path) const override;
    virtual QString materialPathFromId(const MaterialId id) const override;

protected:
    typedef PathManagingObjectStore<RenderSystem::RenderMaterial, RenderSystem::MaterialDefs::MaterialId> BasePathManagingObjectStore;

    virtual void objectCreated(const ObjectId id) override;
    virtual void objectAboutToBeDestroyed(const ObjectId id) override;

private:
    void createDefaultMaterial();
};

#endif // MATERIALSTORE_H
