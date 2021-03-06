#include "resizeablegridlayoutmanager.h"
#include <QGridLayout>
#include <QWidget>
#include "user-interface/arrangeable-tabs/container/resizeablegridlayoutcontainer.h"
#include "resizeablegridelementbutton.h"
#include "calliperutil/debug/debug.h"

namespace
{
    inline int clampAbove(int in, int threshold)
    {
        return in < threshold ? threshold : in;
    }
}

namespace UserInterface
{
    ResizeableGridLayoutManager::ResizeableGridLayoutManager(QGridLayout* gridLayout)
        : m_pGridLayout(gridLayout),
          m_ResizeButtons(),
          m_pModel(new QuadGridLayoutModel(this)),
          m_pAnalyser(new QuadGridLayoutAnalyser(m_pModel))
    {
        m_pGridLayout->setContentsMargins(0,0,0,0);
        m_pGridLayout->setSpacing(0);

        m_pGridLayout->setRowStretch(1, 0);
        m_pGridLayout->setColumnStretch(1, 0);

        m_pGridLayout->setRowMinimumHeight(0, 1);
        m_pGridLayout->setRowMinimumHeight(2, 1);

        m_pGridLayout->setColumnMinimumWidth(0, 1);
        m_pGridLayout->setColumnMinimumWidth(2, 1);
    }

    ResizeableGridLayoutManager::~ResizeableGridLayoutManager()
    {

    }

    void ResizeableGridLayoutManager::addWidget(QWidget *widget, QuadGridLayoutDefs::GridCell cell, Qt::Orientation splitPreference)
    {
        if ( !widget )
            return;

        removeAllWidgetsFromGridLayout();

        ResizeableGridLayoutContainer* container = createContainerForEmbed(cell, splitPreference);
        container->addWidget(widget);

        updateGridLayout();
        updateStretchFactors();
    }

    void ResizeableGridLayoutManager::insertWidget(QWidget *widget, QuadGridLayoutDefs::GridCell cell)
    {
        if ( !widget )
            return;

        removeAllWidgetsFromGridLayout();

        ResizeableGridLayoutContainer* container = createContainerForInsert(cell);
        container->addWidget(widget);

        updateGridLayout();
        updateStretchFactors();
    }

    ResizeableGridLayoutContainer* ResizeableGridLayoutManager::createContainerForEmbed(QuadGridLayoutDefs::GridCell cell, Qt::Orientation splitPreference)
    {
        ResizeableGridLayoutContainer* container = Q_NULLPTR;

        if ( m_pModel->canAddWidget(cell) )
        {
            container = createContainer();
            m_pModel->addWidget(container, cell, splitPreference);
        }
        else
        {
            container = containerAt(cell);
            Q_ASSERT(container);
        }

        return container;
    }

    ResizeableGridLayoutContainer* ResizeableGridLayoutManager::createContainerForInsert(QuadGridLayoutDefs::GridCell cell)
    {
        ResizeableGridLayoutContainer* container = Q_NULLPTR;

        if ( !m_pModel->widgetAt(cell) )
        {
            container = createContainer();
            m_pModel->addWidget(container, cell);
        }
        else
        {
            container = containerAt(cell);
            Q_ASSERT(container);
        }

        return container;
    }

    ResizeableGridLayoutContainer* ResizeableGridLayoutManager::createContainer()
    {
        ResizeableGridLayoutContainer* container = new ResizeableGridLayoutContainer();

        connect(container, &ResizeableGridLayoutContainer::maximizeInvoked, this, &ResizeableGridLayoutManager::maximizeInvoked);
        connect(container, &ResizeableGridLayoutContainer::closeInvoked, this, &ResizeableGridLayoutManager::closeInvoked);
        connect(container, &ResizeableGridLayoutContainer::floatInvoked, this, &ResizeableGridLayoutManager::floatInvoked);

        return container;
    }

    QWidget* ResizeableGridLayoutManager::takeActiveWidget(QuadGridLayoutDefs::GridCell cell, Qt::Orientation mergePreference)
    {
        ResizeableGridLayoutContainer* container = containerAt(cell);
        if ( !container )
            return Q_NULLPTR;

        return removeWidget(container, container->currentWidgetIndex(), mergePreference);
    }

    QWidget* ResizeableGridLayoutManager::widgetAt(QuadGridLayoutDefs::GridCell cell) const
    {
        return m_pModel->widgetAt(cell);
    }

    ResizeableGridLayoutContainer* ResizeableGridLayoutManager::containerAt(QuadGridLayoutDefs::GridCell cell) const
    {
        return qobject_cast<ResizeableGridLayoutContainer*>(m_pModel->widgetAt(cell));
    }

    void ResizeableGridLayoutManager::equaliseCellSizes()
    {
        switch ( m_pModel->widgetCount() )
        {
            case 2:
            {
                if ( m_pAnalyser->majorSplit() == QuadGridLayoutDefs::MajorHorizontal )
                {
                    setStretchFactors(1, 1, 0, 0);
                }
                else
                {
                    setStretchFactors(0, 0, 1, 1);
                }

                break;
            }

            case 3:
            case 4:
            {
                setStretchFactors(1, 1, 1, 1);
                break;
            }

            default:
            {
                break;
            }
        }
    }

    void ResizeableGridLayoutManager::clear()
    {
        removeAllWidgetsFromGridLayout();
        m_pModel->deleteAllWidgets();
    }

    int ResizeableGridLayoutManager::widgetCount() const
    {
        return m_pModel->widgetCount();
    }

    void ResizeableGridLayoutManager::resizeButtonDragged(int deltaX, int deltaY)
    {
        ResizeableGridElementButton* button = qobject_cast<ResizeableGridElementButton*>(sender());
        if ( !button )
            return;

        if ( button->resizeFlags().testFlag(ResizeableGridElementButton::HorizontalResizeFlag) )
        {
            resizeHorizontal(deltaX);
        }

        if ( button->resizeFlags().testFlag(ResizeableGridElementButton::VerticalResizeFlag) )
        {
            resizeVertical(deltaY);
        }

        qDebug() << "Col 0:" << m_pGridLayout->columnStretch(0) << "Col 2:" << m_pGridLayout->columnStretch(2)
                 << "Row 0:" << m_pGridLayout->rowStretch(0) << "Row 2:" << m_pGridLayout->rowStretch(2);
    }

    void ResizeableGridLayoutManager::updateGridLayout()
    {
        switch ( m_pModel->widgetCount() )
        {
            case 1:
            {
                setSingleWidgetLayout();
                break;
            }

            case 2:
            case 3:
            case 4:
            {
                genericAddWidgets();
                break;
            }

            default:
            {
                return;
            }
        }

        addResizeButtons();
    }

    void ResizeableGridLayoutManager::addResizeButtons()
    {
        if ( m_pAnalyser->minorSplit() == QuadGridLayoutDefs::MinorNone )
        {
            addMajorSplitOnly();
        }
        else
        {
            addMajorMinorSplit();
        }
    }

    void ResizeableGridLayoutManager::addMajorSplitOnly()
    {
        switch ( m_pAnalyser->majorSplit() )
        {
            case QuadGridLayoutDefs::MajorHorizontal:
            {
                // 0 0 0
                // - - -
                // 0 0 0
                addResizeButton(1, 0, 1, 3);
                break;
            }

            case QuadGridLayoutDefs::MajorVertical:
            {
                // 0 | 0
                // 0 | 0
                // 0 | 0
                addResizeButton(0, 1, 3, 1);
                break;
            }

            case QuadGridLayoutDefs::MajorBoth:
            {
                // 0 | 0
                // - + -
                // 0 | 0
                addResizeButton(0, 1, 1, 1);
                addResizeButton(1, 0, 1, 1);
                addResizeButton(1, 1, 1, 1);
                addResizeButton(1, 2, 1, 1);
                addResizeButton(2, 1, 1, 1);
                break;
            }

            default:
            {
                break;
            }
        }
    }

    void ResizeableGridLayoutManager::addMajorMinorSplit()
    {
        switch ( m_pAnalyser->majorSplit() )
        {
            case QuadGridLayoutDefs::MajorHorizontal:
            {
                // 0 0 0
                // - - -
                // 0 0 0
                addResizeButton(1, 0, 1, 1);
                addResizeButton(1, 1, 1, 1);
                addResizeButton(1, 2, 1, 1);
                break;
            }

            case QuadGridLayoutDefs::MajorVertical:
            {
                // 0 | 0
                // 0 | 0
                // 0 | 0
                addResizeButton(0, 1, 1, 1);
                addResizeButton(1, 1, 1, 1);
                addResizeButton(2, 1, 1, 1);
                break;
            }

            default:
            {
                Q_ASSERT(false);
                break;
            }
        }

        switch ( m_pAnalyser->minorSplit() )
        {
            case QuadGridLayoutDefs::MinorNorth:
            {
                addResizeButton(0, 1, 1, 1);
                break;
            }

            case QuadGridLayoutDefs::MinorEast:
            {
                addResizeButton(1, 2, 1, 1);
                break;
            }

            case QuadGridLayoutDefs::MinorSouth:
            {
                addResizeButton(2, 1, 1, 1);
                break;
            }

            case QuadGridLayoutDefs::MinorWest:
            {
                addResizeButton(1, 0, 1, 1);
                break;
            }

            default:
            {
                Q_ASSERT(false);
                break;
            }
        }
    }

    void ResizeableGridLayoutManager::removeAllWidgetsFromGridLayout()
    {
        QLayoutItem *child;
        while ( (child = m_pGridLayout->takeAt(0)) != Q_NULLPTR )
        {
            // Only delete buttons - the other widgets are still held
            // in the model and may be re-inserted later.
            ResizeableGridElementButton* button = qobject_cast<ResizeableGridElementButton*>(child->widget());
            if ( button )
            {
                delete button;
            }

            delete child;
        }

        m_ResizeButtons.clear();
    }

    void ResizeableGridLayoutManager::setSingleWidgetLayout()
    {
        addFullSpanWidgetToLayout(m_pModel->widgetAt(QuadGridLayoutDefs::NorthWest));
    }

    void ResizeableGridLayoutManager::genericAddWidgets()
    {
        QList<QWidget*> widgetList = m_pModel->widgets();
        foreach ( QWidget* widget, widgetList )
        {
            addWidgetToLayout(widget);
        }
    }

    void ResizeableGridLayoutManager::addWidgetToLayout(QWidget *widget)
    {
        QuadGridLayoutModel::GridCellList cellList = m_pModel->widgetCells(widget);
        QuadGridLayoutPoint point(QuadGridLayoutModel::lowestGridCell(cellList));
        QuadGridLayoutDefs::WidgetSpan span = m_pModel->widgetSpan(widget);

        addWidgetToLayout(widget, point, span);
    }

    void ResizeableGridLayoutManager::addFullSpanWidgetToLayout(QWidget *widget)
    {
        addWidgetToLayout(widget, 0, 0, 3, 3);
    }

    void ResizeableGridLayoutManager::addWidgetToLayout(QWidget *widget, const QuadGridLayoutPoint &point, QuadGridLayoutDefs::WidgetSpan span)
    {
        // The grid layout rows/cols are 0 or 2, so we need to adjust this.
        int row = point.y() * 2;
        int col = point.x() * 2;
        int rowSpan = span == QuadGridLayoutDefs::VerticalSpan ? 3 : 1;
        int colSpan = span == QuadGridLayoutDefs::HorizontalSpan ? 3 : 1;

        addWidgetToLayout(widget, row, col, rowSpan, colSpan);
    }

    void ResizeableGridLayoutManager::addWidgetToLayout(QWidget *widget, int row, int col, int rowSpan, int colSpan)
    {
        m_pGridLayout->addWidget(widget, row, col, rowSpan, colSpan);
    }

    void ResizeableGridLayoutManager::addResizeButton(int row, int column, int rowSpan, int colSpan)
    {
        ResizeableGridElementButton::ResizeModeFlags flags = ResizeableGridElementButton::NoResizeFlag;

        if ( column == 1 )
        {
            flags |= ResizeableGridElementButton::HorizontalResizeFlag;
        }

        if ( row == 1 )
        {
            flags |= ResizeableGridElementButton::VerticalResizeFlag;
        }

        ResizeableGridElementButton* btn = new ResizeableGridElementButton(flags);
        btn->setRowIndex(row);
        btn->setColumnIndex(column);
        m_pGridLayout->addWidget(btn, row, column, rowSpan, colSpan);
        connect(btn, SIGNAL(mouseMoved(int,int)), this, SLOT(resizeButtonDragged(int,int)));

        m_ResizeButtons.append(btn);
    }

    void ResizeableGridLayoutManager::updateStretchFactors()
    {
        switch ( m_pModel->widgetCount() )
        {
            case 1:
            {
                setSingleWidgetStretchFactors();
                break;
            }

            case 2:
            {
                setDualWidgetStretchFactors();
                break;
            }

            case 3:
            {
                setTripleWidgetStretchFactors();
                break;
            }

            default:
            {
                break;
            }
        }
    }

    void ResizeableGridLayoutManager::setSingleWidgetStretchFactors()
    {
        setStretchFactors(1, 0, 1, 0);
    }

    void ResizeableGridLayoutManager::setDualWidgetStretchFactors()
    {
        if ( m_pAnalyser->majorSplit() == QuadGridLayoutDefs::MajorVertical )
        {
            setStretchFactors(1, 0, 1, 1);
        }
        else
        {
            setStretchFactors(1, 1, 1, 0);
        }
    }

    void ResizeableGridLayoutManager::setTripleWidgetStretchFactors()
    {
        switch ( m_pAnalyser->minorSplit() )
        {
            case QuadGridLayoutDefs::MinorNorth:
            case QuadGridLayoutDefs::MinorSouth:
            {
                m_pGridLayout->setColumnStretch(0, 1);
                m_pGridLayout->setColumnStretch(2, 1);
                break;
            }

            case QuadGridLayoutDefs::MinorWest:
            case QuadGridLayoutDefs::MinorEast:
            {
                m_pGridLayout->setRowStretch(0, 1);
                m_pGridLayout->setRowStretch(2, 1);
                break;
            }

            default:
            {
                break;
            }
        }
    }

    void ResizeableGridLayoutManager::setStretchFactors(int row0, int row2, int col0, int col2)
    {
        m_pGridLayout->setRowStretch(0, row0);
        m_pGridLayout->setRowStretch(2, row2);
        m_pGridLayout->setColumnStretch(0, col0);
        m_pGridLayout->setColumnStretch(2, col2);
    }

    void ResizeableGridLayoutManager::resizeHorizontal(int delta)
    {
        QRect leftRect = m_pGridLayout->cellRect(0,0);
        QRect rightRect = m_pGridLayout->cellRect(0,2);

        if ( !leftRect.isValid() || !rightRect.isValid() )
            return;

        int left = clampAbove(leftRect.width() + delta, 1);
        int right = clampAbove(rightRect.width() - delta, 1);

        m_pGridLayout->setColumnStretch(0, left);
        m_pGridLayout->setColumnStretch(2, right);
    }

    void ResizeableGridLayoutManager::resizeVertical(int delta)
    {
        QRect topRect = m_pGridLayout->cellRect(0,0);
        QRect bottomRect = m_pGridLayout->cellRect(2,0);

        if ( !topRect.isValid() || !bottomRect.isValid() )
            return;

        int top = clampAbove(topRect.height() + delta, 1);
        int bottom = clampAbove(bottomRect.height() - delta, 1);

        m_pGridLayout->setRowStretch(0, top);
        m_pGridLayout->setRowStretch(2, bottom);
    }

    void ResizeableGridLayoutManager::maximizeInvoked(int itemId)
    {
        ResizeableGridLayoutContainer* container = qobject_cast<ResizeableGridLayoutContainer*>(sender());
        if ( !container )
            return;

        QuadGridLayoutModel::GridCellList cells = m_pModel->widgetCells(container);
        if ( cells.isEmpty() )
            return;

        container->setCurrentWidgetIndex(itemId);
        maximize(QuadGridLayoutModel::lowestGridCell(cells));
    }

    void ResizeableGridLayoutManager::maximize(QuadGridLayoutDefs::GridCell cell)
    {
        // This is a pretty awful way to do maximisation, but setting the non-maximised
        // rows/columns to 0 doesn't always squash them down. Instead, we need to set
        // them to 1 and set the maximised areas to something much larger.
        static const int LARGE_STRETCH = 65535;

        QuadGridLayoutPoint point(cell);
        QuadGridLayoutPoint opposite = point.diagonalNeighbour();
        m_pGridLayout->setRowStretch(point.y() * 2, LARGE_STRETCH);
        m_pGridLayout->setRowStretch(opposite.y() * 2, 1);
        m_pGridLayout->setColumnStretch(point.x() * 2, LARGE_STRETCH);
        m_pGridLayout->setColumnStretch(opposite.x() * 2, 1);
    }

    void ResizeableGridLayoutManager::closeInvoked(int itemId)
    {
        ResizeableGridLayoutContainer* container = qobject_cast<ResizeableGridLayoutContainer*>(sender());
        if ( !container )
            return;

        QWidget* widget = removeWidget(container, itemId, Qt::Horizontal);
        delete widget;
    }

    QWidget* ResizeableGridLayoutManager::removeWidget(ResizeableGridLayoutContainer *container, int index, Qt::Orientation mergePreference)
    {
        QuadGridLayoutModel::GridCellList list = m_pModel->widgetCells(container);
        if ( list.isEmpty() )
        {
            Q_ASSERT_X(false, Q_FUNC_INFO, "Expected container to be part of model!");
            return Q_NULLPTR;
        }

        QuadGridLayoutDefs::GridCell cell = QuadGridLayoutModel::lowestGridCell(list);

        QWidget* widget = container->removeWidget(index);

        if ( container->widgetCount() < 1 )
        {
            removeAllWidgetsFromGridLayout();

            CUTL_ASSERT_SUCCESS(m_pModel->removeWidget(cell, mergePreference));

            delete container;
            updateGridLayout();
        }

        return widget;
    }

    void ResizeableGridLayoutManager::floatInvoked(int itemId, bool dragged)
    {
        ResizeableGridLayoutContainer* container = qobject_cast<ResizeableGridLayoutContainer*>(sender());
        if ( !container )
            return;

        QWidget* widget = container->widgetAt(itemId);
        Q_ASSERT(widget);
        QPoint globalPos = widget->mapToGlobal(QPoint(0,0));

        QWidget* removalWidget = removeWidget(container, itemId, Qt::Horizontal);
        Q_ASSERT(widget == removalWidget);

        emit widgetFloated(widget, globalPos, dragged);
    }
}
