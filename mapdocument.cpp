#include "mapdocument.h"
#include "mapscene.h"
#include "scenefactory.h"
#include "sceneobject.h"
#include "tools.h"
#include "inputprocessor.h"
#include "openglrenderer.h"

MapDocument::MapDocument(QObject *parent) : QObject(parent)
{
    m_pScene = SceneFactory::debugScene(this);
    m_pUIScene = SceneFactory::debugUIScene(this);
    m_colBackground = QColor::fromRgb(0xff262626);
    m_colSelected = QColor::fromRgb(0xffff0000);

    m_pInputProcessor = new InputProcessor(this);

    m_iActiveTool = -1;
    createTools();
}

MapDocument::~MapDocument()
{
    qDeleteAll(m_Tools);
    delete m_pScene;
}

MapScene* MapDocument::scene() const
{
    return m_pScene;
}

QColor MapDocument::backgroundColor() const
{
    return m_colBackground;
}

void MapDocument::setBackgroundColor(const QColor &col)
{
    m_colBackground = col;
    m_colBackground.setAlpha(255);
}

const QSet<SceneObject*>& MapDocument::selectedSet() const
{
    return m_SelectedSet;
}

void MapDocument::selectedSetInsert(SceneObject *object)
{
    m_SelectedSet.insert(object);
    BaseTool* t = tool(m_iActiveTool);
    if ( t )
        t->selectedSetChanged();
}

void MapDocument::selectedSetRemove(SceneObject *object)
{
    m_SelectedSet.remove(object);
    BaseTool* t = tool(m_iActiveTool);
    if ( t )
        t->selectedSetChanged();
}

void MapDocument::selectedSetClear()
{
    m_SelectedSet.clear();
    BaseTool* t = tool(m_iActiveTool);
    if ( t )
        t->selectedSetChanged();
}

QColor MapDocument::selectedColor() const
{
    return m_colSelected;
}

void MapDocument::setSelectedColor(const QColor &col)
{
    m_colSelected = col;
}

BaseTool* MapDocument::tool(int index) const
{
    return index >= 0 && index < m_Tools.count() ? m_Tools.at(index) : NULL;
}

BaseTool* MapDocument::tool(const QString &name) const
{
    foreach ( BaseTool* t, m_Tools )
    {
        if ( t->objectName() == name )
            return t;
    }

    return NULL;
}

int MapDocument::toolCount() const
{
    return m_Tools.count();
}

void MapDocument::createTools()
{
    m_Tools.append(new DebugTestTool(this));
}

int MapDocument::activeToolIndex() const
{
    return m_iActiveTool;
}

void MapDocument::setActiveToolIndex(int index)
{
    if ( index == m_iActiveTool )
        return;

    BaseTool* oldTool = activeTool();
    m_iActiveTool = index;
    switchTool(oldTool, activeTool());
}

void MapDocument::switchTool(BaseTool *oldTool, BaseTool *newTool)
{
    if ( oldTool )
    {
        oldTool->deactivate();
    }

    if ( newTool )
    {
        newTool->activate();
    }
}

BaseTool* MapDocument::activeTool() const
{
    return tool(m_iActiveTool);
}

InputProcessor* MapDocument::inputProcessor() const
{
    return m_pInputProcessor;
}

UIScene* MapDocument::uiScene() const
{
    return m_pUIScene;
}

// TODO: Make this faster. Everything is computed on demand, sometimes more than once!
BoundingBox MapDocument::selectedSetBounds() const
{
    BoundingBox bbox;

    for ( QSet<SceneObject*>::const_iterator it = m_SelectedSet.cbegin(); it != m_SelectedSet.cend(); ++it )
    {
        SceneObject* o = *it;
        bbox.unionWith(o->rootToLocal().inverted() * o->computeLocalBounds());
    }

    return bbox;
}
