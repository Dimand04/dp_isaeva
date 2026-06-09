#include "baseeditoritem.h"

BaseEditorItem::BaseEditorItem(QGraphicsItem *parent)
    : QGraphicsObject(parent)
{
    m_id = QUuid::createUuid().toString(QUuid::WithoutBraces);

    setFlags(QGraphicsItem::ItemIsSelectable |
             QGraphicsItem::ItemIsMovable |
             QGraphicsItem::ItemSendsGeometryChanges);
}

BaseEditorItem::~BaseEditorItem()
{
}

QVariant BaseEditorItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged) {
        update();
    }
    else if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        int gridSize = 20;

        qreal x = qRound(newPos.x() / gridSize) * gridSize;
        qreal y = qRound(newPos.y() / gridSize) * gridSize;

        return QPointF(x, y);
    }

    return QGraphicsObject::itemChange(change, value);
}
