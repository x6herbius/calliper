#include "keymap.h"
#include "QEvent"
#include <QKeyEvent>
#include <QtDebug>

namespace Model
{
    KeyMap::KeyMap(QObject *parent) : QObject(parent)
    {

    }

    bool KeyMap::eventFilter(QObject *watched, QEvent *event)
    {
        Q_UNUSED(watched);
        bool press = false;

        switch ( event->type() )
        {
            case QEvent::KeyPress:
                press = true;
                break;

            case QEvent::KeyRelease:
                press = false;
                break;

            default:
                return false;
        }

        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if ( keyEvent->isAutoRepeat() )
        {
            return true;
        }

        int keyCode = static_cast<Qt::Key>(keyEvent->key());
        KeySignalSender* signalSender = keyMap(keyCode);
        if ( !signalSender )
        {
            return false;
        }

        signalSender->triggerKeyEvent(keyCode, press);
        return true;
    }

    KeySignalSender* KeyMap::keyMap(int key) const
    {
        return m_KeyMap.value(key, Q_NULLPTR);
    }

    KeySignalSender* KeyMap::addKeyMap(int key, KeySignalSender::SignalModeFlags signalMode)
    {
        KeySignalSender* sender = m_KeyMap.value(key, Q_NULLPTR);

        if ( !sender )
        {
            sender = new KeySignalSender(signalMode, this);
            m_KeyMap.insert(key, sender);
        }

        return sender;
    }

    void KeyMap::removeKeyMap(int key)
    {
        KeySignalSender* sender = m_KeyMap.value(key, Q_NULLPTR);
        if ( sender )
            delete sender;

        m_KeyMap.remove(key);
    }
}
