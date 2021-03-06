#ifndef KEYMAP_H
#define KEYMAP_H

#include "model_global.h"
#include <QObject>
#include "keysignalsender.h"

namespace Model
{
    class MODELSHARED_EXPORT KeyMap : public QObject
    {
        Q_OBJECT
    public:
        explicit KeyMap(QObject *parent = 0);

        bool eventFilter(QObject *watched, QEvent *event) override;

        KeySignalSender* keyMap(int key) const;
        KeySignalSender* addKeyMap(int key, KeySignalSender::SignalModeFlags signalMode = KeySignalSender::KeyPress | KeySignalSender::KeyRelease);
        void removeKeyMap(int key);

    private:
        typedef QHash<int, KeySignalSender*> KeyMapTable;

        KeyMapTable m_KeyMap;
    };
}

#endif // KEYMAP_H
