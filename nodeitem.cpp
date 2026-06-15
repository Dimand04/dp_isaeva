#include "nodeitem.h"
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QtMath>
#include <QPainter>

NodeItem::NodeItem(const QPointF &pos, QGraphicsItem *parent)
    : BaseEditorItem(parent), m_width(0.2), m_height(0.2), m_isRounded(0), m_isDragging(false)
{
    setPos(pos);
    setZValue(0.6);
    setTransformOriginPoint(10.0, 10.0);
}

QPainterPath NodeItem::currentPath() const
{
    QPainterPath path;
    qreal w = m_width * 100.0;
    qreal h = m_height * 100.0;

    path.moveTo(0, 0);
    path.lineTo(w, 0);

    if (m_isRounded == 1) {
        path.arcTo(QRectF(-w, -h, w * 2, h * 2), 0, -90);
    } else {
        path.lineTo(0, h);
    }

    path.closeSubpath();
    return path;
}

QRectF NodeItem::boundingRect() const
{
    return currentPath().boundingRect().adjusted(-5, -5, 5, 5);
}

QPainterPath NodeItem::shape() const
{
    QPainterPath path;
    path.addPath(currentPath());
    return path;
}

void NodeItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(QColor(230, 230, 230));
    painter->setPen(QPen(QColor(50, 50, 50), 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));

    QPainterPath path = currentPath();
    painter->drawPath(path);

    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(21, 101, 192), 2, Qt::DashLine));
        painter->drawPath(path);
    }
}

void NodeItem::setSide1InMeters(qreal w)
{
    if (qAbs(m_width - w) < 1e-5) return;
    prepareGeometryChange();
    m_width = w;
    update();
    emit itemChanged();
}

void NodeItem::setSide2InMeters(qreal h)
{
    if (qAbs(m_height - h) < 1e-5) return;
    prepareGeometryChange();
    m_height = h;
    update();
    emit itemChanged();
}

void NodeItem::setRounded(int rounded)
{
    if (m_isRounded == rounded) return;
    prepareGeometryChange();
    m_isRounded = rounded;
    update();
    emit itemChanged();
}

void NodeItem::setRotationAngle(qreal angle)
{
    if (qAbs(rotation() - angle) < 1e-5) return;
    setRotation(angle);
    emit itemChanged();
}

qreal NodeItem::side1InMeters() const { return m_width; }
qreal NodeItem::side2InMeters() const { return m_height; }
int NodeItem::isRounded() const { return m_isRounded; }
qreal NodeItem::rotationAngle() const { return rotation(); }

qreal NodeItem::area() const
{
    if (m_isRounded == 1) {
        return (M_PI * m_width * m_height) / 4.0;
    } else {
        return (m_width * m_height) / 2.0;
    }
}

void NodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelected()) {
        m_isDragging = true;
        setCursor(Qt::ClosedHandCursor);
    }
    BaseEditorItem::mousePressEvent(event);
}

void NodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragging) {
        int gridSize = 5;
        QPointF snappedScenePos = event->scenePos();
        snappedScenePos.setX(qRound(snappedScenePos.x() / gridSize) * gridSize);
        snappedScenePos.setY(qRound(snappedScenePos.y() / gridSize) * gridSize);
        setPos(snappedScenePos);
        emit itemChanged();
        return;
    }
    BaseEditorItem::mouseMoveEvent(event);
}

void NodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_isDragging = false;
    setCursor(Qt::ArrowCursor);
    BaseEditorItem::mouseReleaseEvent(event);
}
