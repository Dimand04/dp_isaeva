#include "dooritem.h"
#include "wallitem.h"
#include "windowitem.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QtMath>
#include <QGraphicsScene>

DoorItem::DoorItem(QGraphicsItem *parent)
    : BaseEditorItem(parent), m_hostWall(nullptr), m_width(90.0), m_depth(20.0),
    m_swingType(0), m_distance(0.0)
{
    setZValue(1.1);
}

void DoorItem::setHostWall(WallItem *wall)
{
    m_hostWall = wall;
    updateGeometryToWall();
}

WallItem* DoorItem::hostWall() const
{
    return m_hostWall;
}

QRectF DoorItem::boundingRect() const
{
    qreal halfW = m_width / 2.0;
    qreal halfD = m_depth / 2.0;
    return QRectF(-halfW - 5, -halfD - m_width - 5, m_width + 10, m_depth + m_width * 2 + 10);
}

QPainterPath DoorItem::shape() const
{
    QPainterPath path;
    qreal halfW = m_width / 2.0;
    qreal halfD = m_depth / 2.0;
    path.addRect(-halfW, -halfD, m_width, m_depth);
    return path;
}

void DoorItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    qreal halfW = m_width / 2.0;
    qreal halfD = m_depth / 2.0;

    painter->setBrush(Qt::white);
    painter->setPen(QPen(QColor(50, 50, 50), 2));
    painter->drawRect(QRectF(-halfW, -halfD, m_width, m_depth));

    painter->setPen(QPen(QColor(130, 130, 130), 1));
    if (m_swingType == 0) {
        painter->drawLine(QPointF(-halfW, -halfD), QPointF(-halfW, -halfD - m_width));
        painter->drawArc(QRectF(-halfW - m_width, -halfD - m_width, m_width * 2, m_width * 2), 0, 90 * 16);
    } else if (m_swingType == 1) {
        painter->drawLine(QPointF(halfW, -halfD), QPointF(halfW, -halfD - m_width));
        painter->drawArc(QRectF(halfW - m_width, -halfD - m_width, m_width * 2, m_width * 2), 90 * 16, 90 * 16);
    } else if (m_swingType == 2) {
        painter->drawLine(QPointF(-halfW, halfD), QPointF(-halfW, halfD + m_width));
        painter->drawArc(QRectF(-halfW - m_width, halfD - m_width, m_width * 2, m_width * 2), 270 * 16, 90 * 16);
    } else if (m_swingType == 3) {
        painter->drawLine(QPointF(halfW, halfD), QPointF(halfW, halfD + m_width));
        painter->drawArc(QRectF(halfW - m_width, halfD - m_width, m_width * 2, m_width * 2), 180 * 16, 90 * 16);
    }

    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(21, 101, 192), 2, Qt::DashLine));
        painter->drawRect(QRectF(-halfW - 2, -halfD - 2, m_width + 4, m_depth + 4));
    }
}

void DoorItem::setWidthInMeters(qreal w)
{
    if (qAbs((m_width / 100.0) - w) < 1e-5) return;
    prepareGeometryChange();
    m_width = w * 100.0;
    updateGeometryToWall();
    update();
    emit itemChanged();
}

qreal DoorItem::widthInMeters() const { return m_width / 100.0; }

void DoorItem::setSwingType(int type)
{
    if (m_swingType == type) return;
    prepareGeometryChange();
    m_swingType = type;
    update();
    emit itemChanged();
}

int DoorItem::swingType() const { return m_swingType; }

qreal DoorItem::distanceFromStart() const { return m_distance; }

qreal DoorItem::area() const
{
    if (m_hostWall) {
        return widthInMeters() * (m_hostWall->thicknessInMm() / 1000.0);
    }
    return 0.0;
}

QPointF DoorItem::snapPosition(const QPointF &pos) const
{
    if (!m_hostWall) return pos;

    QPointF p1 = m_hostWall->mapToScene(m_hostWall->line().p1());
    QPointF p2 = m_hostWall->mapToScene(m_hostWall->line().p2());

    QLineF wallLineScene(p1, p2);
    QLineF normal = wallLineScene.normalVector();
    if (normal.length() == 0) return pos;
    normal.setLength(1.0);
    QPointF dir(normal.dx(), normal.dy());

    qreal thickPx = m_hostWall->thicknessInMm() * 0.1;
    QPointF offset(0, 0);
    if (m_hostWall->alignment() == 0) offset = -dir * (thickPx / 2.0);
    else offset = dir * (thickPx / 2.0);

    QPointF p1_offset = p1 + offset;
    QPointF p2_offset = p2 + offset;

    qreal dx = p2_offset.x() - p1_offset.x();
    qreal dy = p2_offset.y() - p1_offset.y();
    qreal length2 = dx * dx + dy * dy;

    if (length2 > 0) {
        qreal wallLengthPx = qSqrt(length2);
        qreal t = ((pos.x() - p1_offset.x()) * dx + (pos.y() - p1_offset.y()) * dy) / length2;
        qreal proposedCenterPx = t * wallLengthPx;
        qreal halfW = m_width / 2.0;

        if (qAbs(proposedCenterPx - wallLengthPx / 2.0) <= 20.0) {
            proposedCenterPx = wallLengthPx / 2.0;
        }

        proposedCenterPx = qMax(halfW, qMin(wallLengthPx - halfW, proposedCenterPx));

        bool valid = true;
        if (scene()) {
            for (QGraphicsItem *item : scene()->items()) {
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
                        if (proposedCenterPx < obsCenterPx) proposedCenterPx = obsCenterPx - minAllowed;
                        else proposedCenterPx = obsCenterPx + minAllowed;
                    }
                }
            }

            if (proposedCenterPx < halfW - 0.1 || proposedCenterPx > wallLengthPx - halfW + 0.1) valid = false;

            if (valid) {
                for (QGraphicsItem *item : scene()->items()) {
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
                    if (obsCenterPx >= 0 && qAbs(proposedCenterPx - obsCenterPx) < (halfW + obsHalfW) - 0.1) {
                        valid = false;
                        break;
                    }
                }
            }
        }

        if (!valid) proposedCenterPx = (m_distance * 100.0) + halfW;

        qreal finalT = proposedCenterPx / wallLengthPx;
        return QPointF(p1_offset.x() + finalT * dx, p1_offset.y() + finalT * dy);
    }
    return pos;
}

QVariant DoorItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && m_hostWall) {
        QPointF p1 = m_hostWall->mapToScene(m_hostWall->line().p1());
        QPointF p2 = m_hostWall->mapToScene(m_hostWall->line().p2());

        QLineF wallLineScene(p1, p2);
        QLineF normal = wallLineScene.normalVector();
        if (normal.length() > 0) {
            normal.setLength(1.0);
            QPointF dir(normal.dx(), normal.dy());
            qreal thickPx = m_hostWall->thicknessInMm() * 0.1;
            QPointF offset(0, 0);
            if (m_hostWall->alignment() == 0) offset = -dir * (thickPx / 2.0);
            else offset = dir * (thickPx / 2.0);

            QPointF p1_offset = p1 + offset;
            qreal distPx = QLineF(p1_offset, pos()).length();
            m_distance = (distPx - (m_width / 2.0)) / 100.0;
            if (m_distance < 0) m_distance = 0.0;
        }
    }
    return BaseEditorItem::itemChange(change, value);
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

    QLineF wallLineScene(p1, p2);
    QLineF normal = wallLineScene.normalVector();
    if (normal.length() == 0) return;
    normal.setLength(1.0);
    QPointF dir(normal.dx(), normal.dy());

    qreal thickPx = m_hostWall->thicknessInMm() * 0.1;
    QPointF offset(0, 0);
    if (m_hostWall->alignment() == 0) offset = -dir * (thickPx / 2.0);
    else offset = dir * (thickPx / 2.0);

    QPointF p1_offset = p1 + offset;
    QPointF p2_offset = p2 + offset;

    qreal dx = p2_offset.x() - p1_offset.x();
    qreal dy = p2_offset.y() - p1_offset.y();
    qreal lengthPx = qSqrt(dx * dx + dy * dy);

    if (lengthPx > 0) {
        qreal targetPx = (m_distance * 100.0) + (m_width / 2.0);
        qreal t = targetPx / lengthPx;
        setPos(p1_offset.x() + t * dx, p1_offset.y() + t * dy);
    }
}

void DoorItem::setDistanceFromStart(qreal distanceMeters)
{
    if (qAbs(m_distance - distanceMeters) < 1e-5) return;
    if (!m_hostWall) return;

    QPointF p1 = m_hostWall->mapToScene(m_hostWall->line().p1());
    QPointF p2 = m_hostWall->mapToScene(m_hostWall->line().p2());

    QLineF wallLineScene(p1, p2);
    QLineF normal = wallLineScene.normalVector();
    if (normal.length() == 0) return;
    normal.setLength(1.0);
    QPointF dir(normal.dx(), normal.dy());

    qreal thickPx = m_hostWall->thicknessInMm() * 0.1;
    QPointF offset(0, 0);
    if (m_hostWall->alignment() == 0) offset = -dir * (thickPx / 2.0);
    else offset = dir * (thickPx / 2.0);

    QPointF p1_offset = p1 + offset;
    QPointF p2_offset = p2 + offset;

    qreal dx = p2_offset.x() - p1_offset.x();
    qreal dy = p2_offset.y() - p1_offset.y();
    qreal lengthPx = qSqrt(dx * dx + dy * dy);

    if (lengthPx > 0) {
        qreal targetPx = (distanceMeters * 100.0) + (m_width / 2.0);
        qreal t = targetPx / lengthPx;
        setPos(p1_offset.x() + t * dx, p1_offset.y() + t * dy);
    }
}
