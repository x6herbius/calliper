#ifndef MODELVIEWFACTORY_H
#define MODELVIEWFACTORY_H

#include "user-interface_global.h"
#include "model/filedatamodels/base/basefiledatamodel.h"
#include "imodelview.h"

namespace UserInterface
{
    class USERINTERFACESHARED_EXPORT ModelViewFactory
    {
    public:
        static IModelView* createView(Model::BaseFileDataModel::ModelType modelType);

    private:
        ModelViewFactory() = delete;
    };
}

#endif // MODELVIEWFACTORY_H
