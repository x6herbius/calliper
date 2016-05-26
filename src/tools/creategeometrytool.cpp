#include "creategeometrytool.h"
#include "mapdocument.h"
#include "mapscene.h"
#include "blockcreationhandle.h"
#include <QMouseEvent>
#include "application.h"
#include "viewport.h"
#include "mainwindow.h"
#include "callipermath.h"
#include "scenecamera.h"
#include "cameralens.h"
#include "mapgrid.h"
#include "crosshair3d.h"

CreateGeometryTool::CreateGeometryTool(MapDocument *document) : BaseTool(CreateGeometryTool::staticName(), document)
{
    m_pManipulator = NULL;
    m_pCrosshair = NULL;
}

CreateGeometryTool::~CreateGeometryTool()
{
    if ( m_bActive )
    {
        deactivate();
    }
}

QString CreateGeometryTool::staticName()
{
    return staticMetaObject.className();
}

void CreateGeometryTool::vActivate()
{
    BaseTool::vActivate();

    m_pManipulator = m_pDocument->scene()->createSceneObject<BlockCreationHandle>(m_pDocument->scene()->root());
    m_pManipulator->setObjectName("_geometryCreationHandle");
    m_pManipulator->setHidden(true);

    m_pCrosshair = m_pDocument->scene()->createSceneObject<Crosshair3D>(m_pDocument->scene()->root());
    m_pCrosshair->setObjectName("_geometryCreationCrosshair");
    m_pCrosshair->setHidden(true);
}

void CreateGeometryTool::vDeactivate()
{
    m_pDocument->scene()->destroySceneObject(m_pManipulator);
    m_pManipulator = NULL;

    m_pDocument->scene()->destroySceneObject(m_pCrosshair);
    m_pCrosshair = NULL;

    BaseTool::vDeactivate();
}

void CreateGeometryTool::vMousePress(QMouseEvent *e)
{
    Viewport* v = application()->mainWindow()->activeViewport();
    SceneCamera* c = v->camera();
    if ( !v || !c )
        return;

    m_bInDrag = true;
    m_PosDragBegin = e->pos();

    SceneObject* target = v->pickObjectFromDepthBuffer(BaseScene::MapSceneFlag, m_PosDragBegin, SceneObject::EditableOnlyMask);
    Ray3D ray = c->frustumRay(m_PosDragBegin, v->size());

    if ( !target )
    {
        if ( !rayIntersectsZ0Plane(c, ray, m_vecDragBegin) )
        {
            m_bInDrag = false;
            return;
        }

        m_vecDragBeginClamped = m_vecDragBegin;
        Math::clampToNearestMultiple(m_vecDragBeginClamped, m_pDocument->scene()->grid()->gridMultiple(), Math::AxisFlagXYZ);

        m_PosDragCurrent = m_PosDragBegin;

        m_vecDragCurrent = m_vecDragBegin;
        m_vecDragCurrent[2] += m_pDocument->scene()->grid()->gridMultiple();
        m_vecDragCurrentClamped = m_vecDragBeginClamped;

        updateManipulatorBounds();
    }
    else
    {
        // TODO
        m_bInDrag = false;
        return;
    }
}

void CreateGeometryTool::vMouseRelease(QMouseEvent *)
{
    endDrag();
}

void CreateGeometryTool::vMouseMove(QMouseEvent *e)
{
    if ( !m_bInDrag )
        return;

    Viewport* v = application()->mainWindow()->activeViewport();
    SceneCamera* c = v->camera();
    if ( !v || !c )
        return;

    m_PosDragCurrent = e->pos();

    SceneObject* target = v->pickObjectFromDepthBuffer(BaseScene::MapSceneFlag, m_PosDragCurrent, SceneObject::EditableOnlyMask);
    Ray3D ray = c->frustumRay(m_PosDragCurrent, v->size());

    if ( !target )
    {
        if ( !rayIntersectsZ0Plane(c, ray, m_vecDragCurrent) )
        {
            // Do nothing.
            return;
        }

        m_vecDragCurrent[2] += m_pDocument->scene()->grid()->gridMultiple();
        m_vecDragCurrentClamped = m_vecDragCurrent;
        Math::clampToNearestMultiple(m_vecDragCurrentClamped, m_pDocument->scene()->grid()->gridMultiple(), Math::AxisFlagXYZ);

        updateManipulatorBounds();
    }
    else
    {
        // TODO
    }

    updateCrosshairVisibility(e->pos());
}

void CreateGeometryTool::vMouseMoveHover(QMouseEvent *e)
{
    if ( m_bInDrag )
        return;

    if ( m_bMouseLookEnabled )
    {
        BaseTool::vMouseMoveHover(e);
    }

    updateCrosshairVisibility(e->pos());
}

void CreateGeometryTool::updateCrosshairVisibility(const QPoint &mousePos)
{
    Viewport* v = application()->mainWindow()->activeViewport();
    SceneCamera* c = v->camera();
    if ( !v || !c )
    {
        m_pCrosshair->setHidden(true);
        return;
    }

    // TODO: Proper ray casting against objects;
    Ray3D ray = c->frustumRay(mousePos, v->size());
    QVector3D intersection;
    if ( !rayIntersectsZ0Plane(c, ray, intersection) )
    {
        m_pCrosshair->setHidden(true);
        return;
    }

    m_pCrosshair->setHidden(false);

    unsigned int gridMultiple = m_pDocument->scene()->grid()->gridMultiple();
    m_pCrosshair->setScale(QVector3D(gridMultiple, gridMultiple, gridMultiple));
    Math::clampToNearestMultiple(intersection, gridMultiple, Math::AxisFlagXYZ);
    m_pCrosshair->setPosition(intersection);
}

void CreateGeometryTool::endDrag()
{
    m_bInDrag = false;
    updateManipulatorBounds(true);

    Viewport* v = application()->mainWindow()->activeViewport();
    if ( !v )
        return;
    updateCrosshairVisibility(v->mapFromGlobal(QCursor::pos()));
}

bool CreateGeometryTool::rayIntersectsZ0Plane(SceneCamera *camera, const Ray3D &ray, QVector3D &intersection)
{
    bool rayIntersectsZ0 = false;
    QVector3D potentialIntersection = ray.parameterise(Math::AxisZ, 0, &rayIntersectsZ0);

    if ( rayIntersectsZ0 )
    {
        // Check if intersection was behind the origin of the ray.
        if ( QVector3D::dotProduct(ray.direction(), potentialIntersection - ray.origin()) <= 0.0f )
        {
            rayIntersectsZ0 = false;
        }

        // Check if the intersection was past the far plane of the camera.
        else if ( QVector3D::dotProduct(camera->forwardVector(), potentialIntersection - camera->position()) >= camera->lens()->farPlane() )
        {
            rayIntersectsZ0 = false;
        }
    }

    if ( !rayIntersectsZ0 )
    {
        return false;
    }

    intersection = potentialIntersection;
    return true;
}

void CreateGeometryTool::updateManipulatorBounds(bool endOfDrag)
{
    m_pManipulator->setHidden(endOfDrag ? BoundingBox(m_vecDragBeginClamped, m_vecDragCurrentClamped).hasZeroVolume() : false);
    m_pManipulator->setBounds(m_vecDragBeginClamped, m_vecDragCurrentClamped);
}
