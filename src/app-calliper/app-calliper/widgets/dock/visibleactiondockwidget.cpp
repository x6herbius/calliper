#include "visibleactiondockwidget.h"
#include <QSignalBlocker>

namespace AppCalliper
{
    VisibleActionDockWidget::VisibleActionDockWidget(const QString &title, QWidget *parent, Qt::WindowFlags flags)
        : QDockWidget(title, parent, flags)
    {
        init();
    }

    VisibleActionDockWidget::VisibleActionDockWidget(QWidget *parent, Qt::WindowFlags flags)
        : QDockWidget(parent, flags)
    {
        init();
    }

    void VisibleActionDockWidget::init()
    {
        connect(this, &VisibleActionDockWidget::visibilityChanged, this, &VisibleActionDockWidget::handleVisibilityChanged);
    }

    QAction* VisibleActionDockWidget::visibilityAction() const
    {
        return m_pAction.data();
    }

    void VisibleActionDockWidget::setVisibilityAction(QAction *action)
    {
        manageConnections(m_pAction.data(), action);
        m_pAction = QPointer<QAction>(action);
    }

    void VisibleActionDockWidget::clearVisibilityAction()
    {
        setVisibilityAction(Q_NULLPTR);
    }

    void VisibleActionDockWidget::manageConnections(QAction *oldAction, QAction *newAction)
    {
        if ( oldAction )
        {
            disconnect(oldAction, &QAction::triggered, this, &VisibleActionDockWidget::actionCheckedStateChanged);
        }

        if ( newAction )
        {
            connect(newAction, &QAction::triggered, this, &VisibleActionDockWidget::actionCheckedStateChanged);
        }
    }

    void VisibleActionDockWidget::handleVisibilityChanged(bool visible)
    {
        if ( !m_pAction )
            return;

        const QSignalBlocker blocker(m_pAction.data());
        Q_UNUSED(blocker);
        m_pAction->setChecked(visible);
    }

    void VisibleActionDockWidget::actionCheckedStateChanged(bool checked)
    {
        const QSignalBlocker blocker(this);
        Q_UNUSED(blocker);
        setVisible(checked);
    }
}
