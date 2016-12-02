#include "mapscene.h"
#include "scenecamera.h"
#include "sceneobject.h"
#include "mapgrid.h"
#include <QJsonArray>
#include "originmarker.h"
#include "sceneobjectfactory.h"

MapScene::MapScene(MapDocument *document) : BaseScene(document)
{
    m_pGrid = nullptr;
    connect(this, &BaseScene::subtreeDestroyed, this, &MapScene::checkIfSceneCleared);
    insertStandardItems();
}

MapScene::~MapScene()
{
}

void MapScene::checkIfSceneCleared(SceneObject *obj)
{
    if ( obj->isRoot() || obj == m_pInternalObjects || obj == m_pGrid )
        m_pGrid = nullptr;
}

BaseScene::SceneType MapScene::type() const
{
    return TypeMapScene;
}

MapGrid* MapScene::grid() const
{
    return m_pGrid;
}

bool MapScene::serialiseToJson(QJsonObject &obj) const
{
    obj.insert(ISerialisable::KEY_IDENTIFIER(), QJsonValue(MapScene::serialiseIdentifier()));

    QJsonObject rootObj;
    serialiseRecursive(m_pRootObject, rootObj);
    obj.insert("root", QJsonValue(rootObj));

    return true;
}

bool MapScene::unserialiseFromJson(const QJsonObject &serialisedData)
{
    if ( !validateIdentifier(serialisedData, MapScene::serialiseIdentifier()) )
        return false;

    QJsonValue vRootObject = serialisedData.value("root");
    if ( !vRootObject.isObject() )
        return false;

    clear();
    bool success = unserialiseRecursive(nullptr, vRootObject.toObject());
    insertStandardItems();
    return success;
}

QString MapScene::serialiseIdentifier() const
{
    return "MapScene";
}

bool MapScene::serialiseRecursive(SceneObject *obj, QJsonObject &jsonParent) const
{
    // Serialise this object.
    if ( !obj->serialiseToJson(jsonParent) )
        return false;

    // If this object has children, serialise them too.
    QList<SceneObject*> childList = obj->children();
    if ( childList.count() > 0 )
    {
        // Create an array to store the children in.
        QJsonArray arrChildren;

        // Serialise each child.
        foreach ( SceneObject* child, childList )
        {
            QJsonObject childObj;
            if ( serialiseRecursive(child, childObj) )
            {
                arrChildren.append(QJsonValue(childObj));
            }
        }

        // Add this array to the JSON parent.
        jsonParent.insert("children", QJsonValue(arrChildren));
    }

    return true;
}

bool MapScene::unserialiseRecursive(SceneObject *parent, const QJsonObject &serialisedData)
{
    SceneObject* obj = nullptr;
    if ( parent )
    {
        // Unserialise the object and attach it to the parent.
        obj = SceneObjectFactory::unserialiseFrom(this, serialisedData, parent);
        if ( !obj )
            return false;
    }
    else
    {
        obj = m_pRootObject;
    }

    // For each of the children in the serialised data, call this recursively.
    QJsonArray arrChildren = serialisedData.value("children").toArray();
    for ( int i = 0; i < arrChildren.count(); i++ )
    {
        unserialiseRecursive(obj, arrChildren.at(i).toObject());
    }

    return true;
}

void MapScene::insertStandardItems()
{
    OriginMarker* o = createSceneObject<OriginMarker>(m_pInternalObjects);
    o->setObjectName("origin");

    MapGrid* grid = createSceneObject<MapGrid>(m_pInternalObjects);
    grid->setObjectName("grid");
}