#include "baseeditoritem.h"
#include <QGraphicsScene>
#include <QtMath>
#include "dimensionitem.h"
#include "dooritem.h"
#include "flooritem.h"
#include "foundationblockitem.h"
#include "nodeitem.h"
#include "objectitem.h"
#include "roofitem.h"
#include "textitem.h"
#include "wallitem.h"
#include "windowitem.h"

BaseEditorItem::BaseEditorItem(QGraphicsItem *parent)
    : QGraphicsObject(parent),
    m_levelId(1),
    m_layerName("Основной"),
    m_height(2.8)
{
    setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsGeometryChanges);
}

QPointF BaseEditorItem::snapPosition(const QPointF &pos) const
{
    int gridSize = 5;
    qreal x = qRound(pos.x() / gridSize) * gridSize;
    qreal y = qRound(pos.y() / gridSize) * gridSize;
    return QPointF(x, y);
}

QVariant BaseEditorItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        return snapPosition(value.toPointF());
    }
    if (change == ItemPositionHasChanged || change == ItemTransformHasChanged) {
        emit itemChanged();
    }
    return QGraphicsObject::itemChange(change, value);
}

void BaseEditorItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        setFlag(QGraphicsItem::ItemIsMovable, isSelected());
    }
    QGraphicsObject::mousePressEvent(event);
}

void BaseEditorItem::setName(const QString &name)
{
    if (m_name == name) return;
    m_name = name;
    emit itemChanged();
}

QString BaseEditorItem::name() const
{
    return m_name;
}

void BaseEditorItem::setLevelId(int levelId)
{
    if (m_levelId == levelId) return;
    m_levelId = levelId;
    emit itemChanged();
}

int BaseEditorItem::levelId() const
{
    return m_levelId;
}

void BaseEditorItem::setLayerName(const QString &layerName)
{
    if (m_layerName == layerName) return;
    m_layerName = layerName;
    emit itemChanged();
}

QString BaseEditorItem::layerName() const
{
    return m_layerName;
}

void BaseEditorItem::setHeight(qreal height)
{
    if (qAbs(m_height - height) < 1e-5) return;
    m_height = height;
    emit itemChanged();
}

qreal BaseEditorItem::height() const
{
    return m_height;
}

QJsonObject BaseEditorItem::toJson() const
{
    QJsonObject json;
    json["class_type"] = metaObject()->className();
    json["name"] = m_name;
    json["level_id"] = m_levelId;
    json["layer_name"] = m_layerName;
    json["pos_x"] = pos().x();
    json["pos_y"] = pos().y();
    json["rotation"] = rotation();
    json["height"] = m_height;
    json["material_id"] = m_materialId;

    return json;
}

void BaseEditorItem::fromJson(const QJsonObject &json)
{
    m_name = json["name"].toString();
    m_levelId = json["level_id"].toInt();
    m_layerName = json["layer_name"].toString();
    setPos(json["pos_x"].toDouble(), json["pos_y"].toDouble());
    setRotation(json["rotation"].toDouble());

    if (json.contains("height")) {
        setHeight(json["height"].toDouble());
    }

    if (json.contains("material_id")) {
        m_materialId = json["material_id"].toInt();
    }
}

BaseEditorItem* BaseEditorItem::createFromJson(const QJsonObject &json)
{
    QString className = json["class_type"].toString();
    BaseEditorItem *item = nullptr;

    if (className == "WallItem") item = new WallItem(QPointF(0,0));
    else if (className == "FoundationBlockItem") item = new FoundationBlockItem(200, 160);
    else if (className == "ObjectItem") item = new ObjectItem();
    else if (className == "WindowItem") item = new WindowItem();
    else if (className == "DoorItem") item = new DoorItem();
    else if (className == "FloorItem") item = new FloorItem();
    else if (className == "DimensionItem") item = new DimensionItem();
    else if (className == "NodeItem") item = new NodeItem(QPointF(0,0));
    else if (className == "TextItem") item = new TextItem();
    else if (className == "RoofItem") item = new RoofItem();

    if (item) {
        item->fromJson(json);
    }
    return item;
}
