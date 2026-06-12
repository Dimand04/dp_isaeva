#include "windowitem.h"
#include "wallitem.h"
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QtMath>

WindowItem::WindowItem(qreal width, WallItem *hostWall, QGraphicsItem *parent)
    : BaseEditorItem(parent), m_width(width), m_hostWall(hostWall), m_isDragging(false)
{
    setZValue(1.0);
    if (m_hostWall) {
        m_depth = m_hostWall->thicknessInMm() * 0.1;
    } else {
        m_depth = 20.0;
    }
}

QRectF WindowItem::boundingRect() const
{
    return QRectF(-m_width / 2, -m_depth / 2, m_width, m_depth).adjusted(-2, -2, 2, 2);
}

void WindowItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(Qt::white);
    painter->setPen(Qt::NoPen);
    QRectF rect(-m_width / 2, -m_depth / 2, m_width, m_depth);
    painter->drawRect(rect);

    painter->setPen(QPen(QColor(50, 50, 50), 1));
    painter->drawLine(-m_width / 2, -m_depth / 2, -m_width / 2, m_depth / 2);
    painter->drawLine(m_width / 2, -m_depth / 2, m_width / 2, m_depth / 2);

    painter->drawLine(-m_width / 2, -2, m_width / 2, -2);
    painter->drawLine(-m_width / 2, 2, m_width / 2, 2);

    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(21, 101, 192), 2, Qt::DashLine));
        painter->drawRect(rect);
    }
}

void WindowItem::setWidthInMeters(qreal widthMeters)
{
    prepareGeometryChange();
    m_width = widthMeters * 100.0;
    update();
    emit itemChanged();
}

void WindowItem::setElevation(qreal elevation)
{
    m_elevation = elevation;
    emit itemChanged();
}

void WindowItem::setProfileType(int index)
{
    m_profileType = index;
    emit itemChanged();
}

qreal WindowItem::widthInMeters() const
{
    return m_width / 100.0;
}

qreal WindowItem::elevation() const
{
    return m_elevation;
}

int WindowItem::profileType() const
{
    return m_profileType;
}
qreal WindowItem::distanceFromStart() const
{
    if (!m_hostWall) return 0.0;

    QPointF p1 = m_hostWall->mapToScene(m_hostWall->line().p1());
    QLineF distLine(p1, pos());

    return distLine.length() / 100.0;
}

QVariant WindowItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged) {
        update();
    }
    return QGraphicsObject::itemChange(change, value);
}

void WindowItem::snapToPos(const QPointF &pt)
{
    if (!m_hostWall) return;

    QPointF p1 = m_hostWall->mapToScene(m_hostWall->line().p1());
    QPointF p2 = m_hostWall->mapToScene(m_hostWall->line().p2());

    qreal dx = p2.x() - p1.x();
    qreal dy = p2.y() - p1.y();
    qreal length2 = dx * dx + dy * dy;

    if (length2 > 0) {
        qreal t = ((pt.x() - p1.x()) * dx + (pt.y() - p1.y()) * dy) / length2;
        t = qMax(0.0, qMin(1.0, t));

        qreal wallLengthPx = qSqrt(length2);
        qreal pixelsFromCenter = qAbs(t - 0.5) * wallLengthPx;

        if (pixelsFromCenter <= 20.0) {
            t = 0.5;
        }

        QPointF snappedPt(p1.x() + t * dx, p1.y() + t * dy);
        setPos(snappedPt);

        emit itemChanged();
    }
}

void WindowItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        setCursor(Qt::ClosedHandCursor);
    }
    BaseEditorItem::mousePressEvent(event);
}

void WindowItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragging) {
        snapToPos(event->scenePos());
        return;
    }
    BaseEditorItem::mouseMoveEvent(event);
}

void WindowItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_isDragging = false;
    setCursor(Qt::ArrowCursor);
    BaseEditorItem::mouseReleaseEvent(event);
}
