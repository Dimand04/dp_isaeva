#include "flooritem.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QtMath>

FloorItem::FloorItem(QGraphicsItem *parent)
    : BaseEditorItem(parent), m_isDrawing(true), m_dragIndex(-1)
{
    setZValue(0.1);
}

QRectF FloorItem::boundingRect() const
{
    if (m_polygon.isEmpty()) return QRectF();
    return shape().boundingRect().adjusted(-5, -5, 5, 5);
}

QPainterPath FloorItem::shape() const
{
    QPainterPath path;
    if (m_polygon.isEmpty()) return path;

    path.addPolygon(m_polygon);

    QPainterPathStroker stroker;
    stroker.setWidth(15);
    QPainterPath finalPath = stroker.createStroke(path);

    finalPath.addPath(path);

    for (const QPointF &pt : m_polygon) {
        finalPath.addEllipse(pt, 12.0, 12.0);
    }

    return finalPath;
}

void FloorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_polygon.isEmpty()) return;

    if (m_isDrawing) {
        painter->setBrush(QColor(200, 200, 200, 100));
        painter->setPen(QPen(QColor(100, 100, 100), 2, Qt::DashLine));
        painter->drawPolygon(m_polygon);

        painter->setBrush(Qt::white);
        painter->setPen(QPen(Qt::black, 1));
        for (const QPointF &pt : m_polygon) {
            painter->drawEllipse(pt, 3, 3);
        }
    } else {
        painter->setBrush(QColor(230, 225, 215, 180));
        painter->setPen(QPen(QColor(120, 120, 120), 1, Qt::SolidLine));
        painter->drawPolygon(m_polygon);

        if (isSelected()) {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(QColor(21, 101, 192), 2, Qt::DashLine));
            painter->drawPolygon(m_polygon);

            painter->setBrush(Qt::white);
            painter->setPen(QPen(QColor(21, 101, 192), 2));
            for (const QPointF &pt : m_polygon) {
                painter->drawRect(QRectF(pt.x() - 4, pt.y() - 4, 8, 8));
            }
        }
    }
}

void FloorItem::addPoint(const QPointF &scenePos)
{
    prepareGeometryChange();
    if (m_polygon.isEmpty()) {
        setPos(scenePos);
        m_polygon << QPointF(0, 0) << QPointF(0, 0);
    } else {
        m_polygon.last() = mapFromScene(scenePos);
        m_polygon.append(mapFromScene(scenePos));
    }
    update();
}

void FloorItem::setDynamicPoint(const QPointF &scenePos)
{
    if (m_polygon.isEmpty() || !m_isDrawing) return;
    prepareGeometryChange();
    m_polygon.last() = mapFromScene(scenePos);
    update();
}

void FloorItem::finishDrawing()
{
    if (!m_isDrawing) return;

    prepareGeometryChange();
    if (!m_polygon.isEmpty()) {
        m_polygon.removeLast();
    }

    m_isDrawing = false;

    if (m_polygon.size() > 2) {
        QPointF c = m_polygon.boundingRect().center();
        QPolygonF localPoly;
        for (const QPointF &pt : m_polygon) {
            localPoly.append(pt - c);
        }
        m_polygon = localPoly;
        setPos(mapToScene(c));
    }

    update();
    emit itemChanged();
}

void FloorItem::resumeDrawing(const QPointF &cursorScenePos)
{
    if (m_isDrawing) return;
    prepareGeometryChange();
    m_isDrawing = true;
    m_polygon.append(mapFromScene(cursorScenePos));
    update();
}

bool FloorItem::isDrawing() const { return m_isDrawing; }

int FloorItem::polygonSize() const { return m_polygon.size(); }

int FloorItem::vertexAt(const QPointF &localPos) const
{
    for (int i = 0; i < m_polygon.size(); ++i) {
        if (QLineF(localPos, m_polygon[i]).length() <= 12.0) {
            return i;
        }
    }
    return -1;
}

void FloorItem::removePoint(int index)
{
    if (index >= 0 && index < m_polygon.size() && m_polygon.size() > 3) {
        prepareGeometryChange();
        m_polygon.removeAt(index);

        QPointF c = m_polygon.boundingRect().center();
        QPolygonF localPoly;
        for (const QPointF &pt : m_polygon) {
            localPoly.append(pt - c);
        }
        m_polygon = localPoly;
        setPos(mapToScene(c));

        update();
        emit itemChanged();
    }
}

void FloorItem::removeLastPoint()
{
    if (m_isDrawing && m_polygon.size() > 1) {
        prepareGeometryChange();
        m_polygon.removeAt(m_polygon.size() - 2);
        update();
    }
}

qreal FloorItem::area() const
{
    if (m_polygon.size() < 3) return 0.0;
    qreal totalArea = 0;
    int n = m_polygon.size();
    for (int i = 0; i < n; ++i) {
        QPointF p1 = m_polygon[i];
        QPointF p2 = m_polygon[(i + 1) % n];
        totalArea += (p1.x() * p2.y() - p2.x() * p1.y());
    }
    return qAbs(totalArea) / 2.0 / 10000.0;
}

QVariant FloorItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && !m_isDrawing) {
        return snapPosition(value.toPointF());
    }
    return BaseEditorItem::itemChange(change, value);
}

void FloorItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_isDrawing && event->button() == Qt::LeftButton) {
        int idx = vertexAt(event->pos());
        if (idx != -1) {
            setSelected(true);
            m_dragIndex = idx;
            event->accept();
            return;
        }
    }
    BaseEditorItem::mousePressEvent(event);
}

void FloorItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragIndex != -1) {
        QPointF scenePos = event->scenePos();
        QPointF snappedScene = snapPosition(scenePos);

        prepareGeometryChange();
        m_polygon[m_dragIndex] = mapFromScene(snappedScene);
        update();
        emit itemChanged();
        return;
    }
    BaseEditorItem::mouseMoveEvent(event);
}

void FloorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragIndex != -1) {
        m_dragIndex = -1;

        prepareGeometryChange();
        QPointF c = m_polygon.boundingRect().center();
        QPolygonF localPoly;
        for (const QPointF &pt : m_polygon) {
            localPoly.append(pt - c);
        }
        m_polygon = localPoly;
        setPos(mapToScene(c));

        update();
        emit itemChanged();

        event->accept();
        return;
    }
    BaseEditorItem::mouseReleaseEvent(event);
}
