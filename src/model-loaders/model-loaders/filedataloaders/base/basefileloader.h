#ifndef BASEFILELOADER_H
#define BASEFILELOADER_H

#include "model-loaders_global.h"
#include <QObject>

namespace Model
{
    class BaseFileDataModel;
}

namespace ModelLoaders
{
    class MODELLOADERSSHARED_EXPORT BaseFileLoader
    {
        Q_GADGET
    public:
        enum LoaderType
        {
            UnknownLoader = 0,
            VmfLoader
        };

        enum SuccessCode
        {
            Failure = 0,
            PartialSuccess,
            Success,
        };

        virtual ~BaseFileLoader();

        virtual LoaderType type() const = 0;
        bool isValid() const;

        Model::BaseFileDataModel* dataModel() const;
        virtual bool setDataModel(Model::BaseFileDataModel* model) = 0;

        virtual SuccessCode load(const QString& filePath, QString* errorString = Q_NULLPTR) = 0;
        virtual SuccessCode save(const QString& filePath, QString* errorString = Q_NULLPTR) = 0;

    protected:
        BaseFileLoader();

        Model::BaseFileDataModel* m_pDataModel;
    };
}

#endif // BASEFILELOADER_H
