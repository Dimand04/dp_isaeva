#include "dooritem.h"
#include "wallitem.h"
#include "windowitem.h"
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QtMath>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QPainterPathStroker>

DoorItem::DoorItem(qreal width, WallItem *hostWall, QGraphicsItem *parent)
    : BaseEditorItem(parent), m_width(width), m_hostWall(hostWall), m_isDragging(false), m_distance(0.0), m_swingType(0)
{
    setZValue(1.1);
    if (m_hostWall) {
        m_depth = m_hostWall->thicknessInMm() * 0.1;
        connect(m_hostWall, &BaseEditorItem::itemChanged, this, &DoorItem::updateGeometryToWall);
    } else {
        m_depth = 20.0;
    }
}

QRectF DoorItem::boundingRect() const
{
    return QRectF(-m_width / 2 - m_width, -m_depth / 2 - m_width, m_width * 3, m_depth + m_width * 2).adjusted(-2, -2, 2, 2);
}

QPainterPath DoorItem::shape() const
{
    QPainterPath path;

    path.addRect(-m_width / 2, -m_depth / 2, m_width, m_depth);

    QPainterPath arcPath;
    qreal w = m_width;
    qreal d = m_depth / 2;

    if (m_swingType == 0) {
        arcPath.moveTo(-w / 2, d);
        arcPath.lineTo(-w / 2, d + w);
        arcPath.arcTo(-w / 2 - w, d - w, w * 2, w * 2, 0, -90);
    } else if (m_swingType == 1) {
        arcPath.moveTo(w / 2, d);
        arcPath.lineTo(w / 2, d + w);
        arcPath.arcTo(w / 2 - w, d - w, w * 2, w * 2, 180, 90);
    } else if (m_swingType == 2) {
        arcPath.moveTo(-w / 2, -d);
        arcPath.lineTo(-w / 2, -d - w);
        arcPath.arcTo(-w / 2 - w, -d - w, w * 2, w * 2, 0, 90);
    } else if (m_swingType == 3) {
        arcPath.moveTo(w / 2, -d);
        arcPath.lineTo(w / 2, -d - w);
        arcPath.arcTo(w / 2 - w, -d - w, w * 2, w * 2, 180, -90);
    }

    QPainterPathStroker stroker;
    stroker.setWidth(10);

    return path.united(stroker.createStroke(arcPath));
}

void DoorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(Qt::white);
    painter->setPen(Qt::NoPen);
    painter->drawRect(-m_width / 2, -m_depth / 2, m_width, m_depth);

    painter->setPen(QPen(QColor(50, 50, 50), 1));
    painter->drawLine(-m_width / 2, -m_depth / 2, -m_width / 2, m_depth / 2);
    painter->drawLine(m_width / 2, -m_depth / 2, m_width / 2, m_depth / 2);

    qreal w = m_width;
    qreal d = m_depth / 2;

    painter->setPen(QPen(QColor(50, 50, 50), 1, Qt::SolidLine));

    if (m_swingType == 0) {
        painter->drawLine(-w / 2, d, -w / 2, d + w);
        painter->drawArc(-w / 2 - w, d - w, w * 2, w * 2, 0 * 16, -90 * 16);
    } else if (m_swingType == 1) {
        painter->drawLine(w / 2, d, w / 2, d + w);
        painter->drawArc(w / 2 - w, d - w, w * 2, w * 2, 180 * 16, 90 * 16);
    } else if (m_swingType == 2) {
        painter->drawLine(-w / 2, -d, -w / 2, -d - w);
        painter->drawArc(-w / 2 - w, -d - w, w * 2, w * 2, 0 * 16, 90 * 16);
    } else if (m_swingType == 3) {
        painter->drawLine(w / 2, -d, w / 2, -d - w);
        painter->drawArc(w / 2 - w, -d - w, w * 2, w * 2, 180 * 16, -90 * 16);
    }

    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(21, 101, 192), 2, Qt::DashLine));
        painter->drawRect(-m_width / 2, -m_depth / 2, m_width, m_depth);
    }
}

void DoorItem::setWidthInMeters(qreal widthMeters)
{
    qreal newWidth = widthMeters * 100.0;
    if (qAbs(m_width - newWidth) < 1e-5) return;

    prepareGeometryChange();
    m_width = newWidth;
    snapToPos(pos());
    update();
    emit itemChanged();
}

void DoorItem::setSwingType(int type)
{
    if (m_swingType == type) return;
    prepareGeometryChange();
    m_swingType = type;
    update();
    emit itemChanged();
}

qreal DoorItem::widthInMeters() const
{
    return m_width / 100.0;
}

int DoorItem::swingType() const
{
    return m_swingType;
}

qreal DoorItem::distanceFromStart() const
{
    return m_distance;
}

void DoorItem::setDistanceFromStart(qreal distanceMeters)
{
    if (qAbs(m_distance - distanceMeters) < 1e-5) return;
    if (!m_hostWall) return;

    QPointF p1 = m_hostWall->mapToScene(m_hostWall->line().p1());
    QPointF p2 = m_hostWall->mapToScene(m_hostWall->line().p2());

    qreal dx = p2.x() - p1.x();
    qreal dy = p2.y() - p1.y();
    qreal lengthPx = qSqrt(dx * dx + dy * dy);

    if (lengthPx > 0) {
        qreal targetPx = (distanceMeters * 100.0) + (m_width / 2.0);
        qreal t = targetPx / lengthPx;

        QPointF targetPt(p1.x() + t * dx, p1.y() + t * dy);
        snapToPos(targetPt);
    }
}

void DoorItem::updateGeometryToWall()
{
    if (!m_hostWall) return;

    qreal newDepth = m_hostWall->thicknessInMm() * 0.1;
    if (qAbs(m_depth - newDepth) > 1e-5) {
        prepareGeometryChange();
        m_depth = newDepth;
    }

    setRotation(-m_hostWall->line().angle());

    QPointF p1 = m_hostWall->mapToScene(m_hostWall->line().p1());
    QPointF p2 = m_hostWall->mapToScene(m_hostWall->line().p2());
    qreal dx = p2.x() - p1.x();
    qreal dy = p2.y() - p1.y();
    qreal lengthPx = qSqrt(dx * dx + dy * dy);

    if (lengthPx > 0) {
        qreal targetPx = (m_distance * 100.0) + (m_width / 2.0);
        qreal t = targetPx / lengthPx;
        QPointF targetPt(p1.x() + t * dx, p1.y() + t * dy);
        snapToPos(targetPt);
    }
}

QVariant DoorItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedHasChanged) {
        update();
    }
    return QGraphicsObject::itemChange(change, value);
}

void DoorItem::snapToPos(const QPointF &pt)
{
    if (!m_hostWall) return;

    QPointF p1 = m_hostWall->mapToScene(m_hostWall->line().p1());
    QPointF p2 = m_hostWall->mapToScene(m_hostWall->line().p2());

    qreal dx = p2.x() - p1.x();
    qreal dy = p2.y() - p1.y();
    qreal length2 = dx * dx + dy * dy;

    if (length2 > 0) {
        qreal wallLengthPx = qSqrt(length2);
        qreal t = ((pt.x() - p1.x()) * dx + (pt.y() - p1.y()) * dy) / length2;
        qreal proposedCenterPx = t * wallLengthPx;
        qreal halfW = m_width / 2.0;

        if (qAbs(proposedCenterPx - wallLengthPx / 2.0) <= 20.0) {
            proposedCenterPx = wallLengthPx / 2.0;
        }

        proposedCenterPx = qMax(halfW, qMin(wallLengthPx - halfW, proposedCenterPx));

        bool valid = true;
        if (scene()) {
            QList<QGraphicsItem*> items = scene()->items();
            for (QGraphicsItem *item : items) {
                if (item == this) continue;

                qreal obsCenterPx = -1;
                qreal obsHalfW = 0;

                if (WindowItem *w = dynamic_cast<WindowItem*>(item)) {
                    if (w->hostWall() == m_hostWall) {
                        obsHalfW = w->widthInMeters() * 100.0 / 2.0;
                        obsCenterPx = w->distanceFromStart() * 100.0 + obsHalfW;
                    }
                } else if (DoorItem *d = dynamic_cast<DoorItem*>(item)) {
                    if (d->hostWall() == m_hostWall) {
                        obsHalfW = d->widthInMeters() * 100.0 / 2.0;
                        obsCenterPx = d->distanceFromStart() * 100.0 + obsHalfW;
                    }
                }

                if (obsCenterPx >= 0) {
                    qreal distBetween = qAbs(proposedCenterPx - obsCenterPx);
                    qreal minAllowed = halfW + obsHalfW;

                    if (distBetween < minAllowed) {
                        if (proposedCenterPx < obsCenterPx) {
                            proposedCenterPx = obsCenterPx - minAllowed;
                        } else {
                            proposedCenterPx = obsCenterPx + minAllowed;
                        }
                    }
                }
            }

            if (proposedCenterPx < halfW - 0.1 || proposedCenterPx > wallLengthPx - halfW + 0.1) {
                valid = false;
            }

            if (valid) {
                for (QGraphicsItem *item : items) {
                    if (item == this) continue;

                    qreal obsCenterPx = -1;
                    qreal obsHalfW = 0;
                    if (WindowItem *w = dynamic_cast<WindowItem*>(item)) {
                        if (w->hostWall() == m_hostWall) {
                            obsHalfW = w->widthInMeters() * 100.0 / 2.0;
                            obsCenterPx = w->distanceFromStart() * 100.0 + obsHalfW;
                        }
                    } else if (DoorItem *d = dynamic_cast<DoorItem*>(item)) {
                        if (d->hostWall() == m_hostWall) {
                            obsHalfW = d->widthInMeters() * 100.0 / 2.0;
                            obsCenterPx = d->distanceFromStart() * 100.0 + obsHalfW;
                        }
                    }

                    if (obsCenterPx >= 0) {
                        if (qAbs(proposedCenterPx - obsCenterPx) < (halfW + obsHalfW) - 0.1) {
                            valid = false;
                            break;
                        }
                    }
                }
            }
        }

        if (!valid) {
            proposedCenterPx = (m_distance * 100.0) + halfW;
        }

        qreal finalT = proposedCenterPx / wallLengthPx;
        QPointF snappedPt(p1.x() + finalT * dx, p1.y() + finalT * dy);
        setPos(snappedPt);

        m_distance = (proposedCenterPx - halfW) / 100.0;
        if (m_distance < 0) m_distance = 0.0;

        emit itemChanged();
    }
}

void DoorItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        setCursor(Qt::ClosedHandCursor);
    }
    BaseEditorItem::mousePressEvent(event);
}

void DoorItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragging) {
        snapToPos(event->scenePos());
        return;
    }
    BaseEditorItem::mouseMoveEvent(event);
}

void DoorItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_isDragging = false;
    setCursor(Qt::ArrowCursor);
    BaseEditorItem::mouseReleaseEvent(event);
}

qreal DoorItem::area() const
{
    if (m_hostWall) {
        return widthInMeters() * (m_hostWall->thicknessInMm() / 1000.0);
    }
    return 0.0;
}
