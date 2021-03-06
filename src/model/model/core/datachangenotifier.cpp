#include "datachangenotifier.h"

namespace Model
{
    DataChangeNotifier::DataChangeNotifier(QObject *parent) : QObject(parent),
        m_bDataChanged(false)
    {

    }

    bool DataChangeNotifier::dataHasChanged() const
    {
        return m_bDataChanged;
    }

    void DataChangeNotifier::notifyDataChanged(int propertyIndex)
    {
        m_bDataChanged = true;
        emit dataChanged(propertyIndex);
    }

    void DataChangeNotifier::notifyDataReset()
    {
        if ( !m_bDataChanged )
            return;

        m_bDataChanged = false;
        emit dataReset();
    }

    void DataChangeNotifier::connectDataChangedSignals(DataChangeNotifier *source)
    {
        connect(source, &DataChangeNotifier::dataChanged, this, &DataChangeNotifier::notifyDataChanged);
        connect(this, &DataChangeNotifier::dataReset, source, &DataChangeNotifier::notifyDataReset);
    }
}
