#include "baseeditoritem.h"
#include <QGraphicsScene>
#include <QtMath>

BaseEditorItem::BaseEditorItem(QGraphicsItem *parent)
    : QGraphicsObject(parent)
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
