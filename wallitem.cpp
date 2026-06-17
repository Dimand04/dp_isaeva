#include "wallitem.h"
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QtMath>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QPainterPathStroker>
#include "windowitem.h"
#include "dooritem.h"
#include <QPainter>
#include <QJsonObject>

WallItem::WallItem(const QPointF &startPoint, QGraphicsItem *parent)
    : BaseEditorItem(parent), m_thickness(20.0), m_isUpdating(false), m_state(WallStateNone)
{
    setAcceptHoverEvents(true);
    setZValue(0.5);
    m_line.setP1(QPointF(0, 0));
    m_line.setP2(QPointF(0, 0));
    setPos(startPoint);
    updatePolygon();
}

void WallItem::setEndPoint(const QPointF &endPoint)
{
    prepareGeometryChange();
    m_line.setP2(mapFromScene(endPoint));
    updatePolygon();
    update();
    emit itemChanged();
}

QRectF WallItem::startHandle() const
{
    return QRectF(-5, -5, 10, 10);
}

QRectF WallItem::endHandle() const
{
    return QRectF(m_line.p2().x() - 5, m_line.p2().y() - 5, 10, 10);
}

QRectF WallItem::boundingRect() const
{
    return m_polygon.boundingRect().adjusted(-10, -10, 10, 10);
}

QPainterPath WallItem::shape() const
{
    QPainterPath path;
    path.addPolygon(m_polygon);
    QPainterPathStroker stroker;
    stroker.setWidth(10);
    return path.united(stroker.createStroke(path));
}

void WallItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(QColor(230, 230, 230));
    painter->setPen(QPen(QColor(50, 50, 50), 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
    painter->drawPolygon(m_polygon);

    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(21, 101, 192), 2, Qt::DashLine));
        painter->drawPolygon(m_polygon);

        painter->setBrush(Qt::white);
        painter->setPen(QPen(QColor(21, 101, 192), 1));
        painter->drawRect(startHandle());
        painter->drawRect(endHandle());
    }
}

void WallItem::setLengthInMeters(qreal lengthMeters)
{
    if (qAbs(lengthInMeters() - lengthMeters) < 1e-5) return;

    prepareGeometryChange();
    qreal lengthPx = lengthMeters * 100.0;
    qreal angleRad = qDegreesToRadians(m_line.angle());
    qreal newX = m_line.p1().x() + lengthPx * qCos(angleRad);
    qreal newY = m_line.p1().y() - lengthPx * qSin(angleRad);
    m_line.setP2(QPointF(newX, newY));
    updatePolygon();
    update();
    emit itemChanged();
}

void WallItem::setThicknessInMm(qreal thicknessMm)
{
    if (qAbs(thicknessInMm() - thicknessMm) < 1e-5) return;

    prepareGeometryChange();
    m_thickness = thicknessMm * 0.1;
    updatePolygon();
    update();
    emit itemChanged();
}

void WallItem::setAngleInDegrees(qreal angleDeg)
{
    if (qAbs(angleInDegrees() - angleDeg) < 1e-5) return;

    prepareGeometryChange();
    m_line.setAngle(angleDeg);
    updatePolygon();
    update();
    emit itemChanged();
}

qreal WallItem::lengthInMeters() const
{
    return m_line.length() / 100.0;
}

qreal WallItem::thicknessInMm() const
{
    return m_thickness / 0.1;
}

qreal WallItem::angleInDegrees() const
{
    qreal angle = m_line.angle();
    if (angle < 0) angle += 360;
    return angle;
}

void WallItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (isSelected()) {
        if (startHandle().contains(event->pos()) || endHandle().contains(event->pos())) {
            setCursor(Qt::SizeAllCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
    BaseEditorItem::hoverMoveEvent(event);
}

void WallItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelected()) {
        if (startHandle().contains(event->pos())) {
            m_state = WallStateMoveStart;
            return;
        } else if (endHandle().contains(event->pos())) {
            m_state = WallStateMoveEnd;
            return;
        }
    }
    m_state = WallStateNone;
    BaseEditorItem::mousePressEvent(event);
}

void WallItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    int gridSize = 5;

    if (m_state == WallStateMoveEnd) {
        QPointF snappedScenePos = event->scenePos();
        snappedScenePos.setX(qRound(snappedScenePos.x() / gridSize) * gridSize);
        snappedScenePos.setY(qRound(snappedScenePos.y() / gridSize) * gridSize);

        if (event->modifiers() & Qt::ShiftModifier) {
            QPointF startP = pos();
            if (qAbs(snappedScenePos.x() - startP.x()) > qAbs(snappedScenePos.y() - startP.y())) {
                snappedScenePos.setY(startP.y());
            } else {
                snappedScenePos.setX(startP.x());
            }
        }

        prepareGeometryChange();
        m_line.setP2(mapFromScene(snappedScenePos));

        updatePolygon();
        if (scene()) scene()->update();
        emit itemChanged();

    } else if (m_state == WallStateMoveStart) {
        QPointF oldEndScene = mapToScene(m_line.p2());

        QPointF snappedStartScene = event->scenePos();
        snappedStartScene.setX(qRound(snappedStartScene.x() / gridSize) * gridSize);
        snappedStartScene.setY(qRound(snappedStartScene.y() / gridSize) * gridSize);

        if (event->modifiers() & Qt::ShiftModifier) {
            if (qAbs(snappedStartScene.x() - oldEndScene.x()) > qAbs(snappedStartScene.y() - oldEndScene.y())) {
                snappedStartScene.setY(oldEndScene.y());
            } else {
                snappedStartScene.setX(oldEndScene.x());
            }
        }

        prepareGeometryChange();
        setPos(snappedStartScene);
        m_line.setP2(mapFromScene(oldEndScene));

        updatePolygon();
        if (scene()) scene()->update();
        emit itemChanged();

    } else {
        BaseEditorItem::mouseMoveEvent(event);
    }
}

void WallItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_state = WallStateNone;
    BaseEditorItem::mouseReleaseEvent(event);
}

QVariant WallItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged || change == ItemTransformHasChanged) {
        updatePolygon();
    }
    return BaseEditorItem::itemChange(change, value);
}

void WallItem::updatePolygon()
{
    if (m_isUpdating) return;
    m_isUpdating = true;

    QPointF p1 = m_line.p1();
    QPointF p2 = m_line.p2();

    QLineF axis = m_line;
    QLineF normal = axis.normalVector();
    if (normal.length() == 0) { m_isUpdating = false; return; }
    normal.setLength(1.0);
    QPointF dir(normal.dx(), normal.dy());

    qreal leftDist = 0.0;
    qreal rightDist = 0.0;

    if (m_alignment == AlignLeft) {
        leftDist = 0.0;
        rightDist = m_thickness;
    } else {
        leftDist = m_thickness;
        rightDist = 0.0;
    }

    QPointF p1L = p1 + dir * leftDist;
    QPointF p1R = p1 - dir * rightDist;
    QPointF p2L = p2 + dir * leftDist;
    QPointF p2R = p2 - dir * rightDist;

    prepareGeometryChange();
    m_polygon.clear();
    m_polygon << p1L << p2L << p2R << p1R;

    m_isUpdating = false;

    updateAttachedItems();
}

qreal WallItem::calculateExactArea() const
{
    if (m_polygon.isEmpty()) return 0.0;
    qreal area = 0.0;
    int j = m_polygon.count() - 1;
    for (int i = 0; i < m_polygon.count(); i++) {
        area += (m_polygon[j].x() + m_polygon[i].x()) * (m_polygon[j].y() - m_polygon[i].y());
        j = i;
    }
    return qAbs(area / 2.0) / 10000.0;
}

qreal WallItem::netArea() const
{
    qreal grossArea = calculateExactArea();
    qreal deductions = 0.0;

    if (scene()) {
        for (QGraphicsItem *item : scene()->items()) {
            if (WindowItem *w = dynamic_cast<WindowItem*>(item)) {
                if (w->hostWall() == this) {
                    deductions += w->widthInMeters() * (thicknessInMm() / 1000.0);
                }
            } else if (DoorItem *d = dynamic_cast<DoorItem*>(item)) {
                if (d->hostWall() == this) {
                    deductions += d->widthInMeters() * (thicknessInMm() / 1000.0);
                }
            }
        }
    }
    return qMax(0.0, grossArea - deductions);
}

int WallItem::alignment() const
{
    return m_alignment;
}

void WallItem::setAlignment(int alignment)
{
    if (m_alignment == alignment) return;
    m_alignment = alignment;
    updatePolygon();
    update();
    emit itemChanged();
}

void WallItem::updateAttachedItems()
{
    if (!scene()) return;

    for (QGraphicsItem *item : scene()->items()) {
        if (WindowItem *w = dynamic_cast<WindowItem*>(item)) {
            if (w->hostWall() == this) {
                w->updateGeometryToWall();
            }
        } else if (DoorItem *d = dynamic_cast<DoorItem*>(item)) {
            if (d->hostWall() == this) {
                d->updateGeometryToWall();
            }
        }
    }
}

qreal WallItem::grossSurfaceArea() const
{
    return lengthInMeters() * height();
}

qreal WallItem::netSurfaceArea() const
{
    qreal gross = grossSurfaceArea();
    qreal deductions = 0.0;

    if (scene()) {
        for (QGraphicsItem *item : scene()->items()) {
            if (WindowItem *w = dynamic_cast<WindowItem*>(item)) {
                if (w->hostWall() == this) {
                    deductions += w->widthInMeters() * w->height();
                }
            } else if (DoorItem *d = dynamic_cast<DoorItem*>(item)) {
                if (d->hostWall() == this) {
                    deductions += d->widthInMeters() * d->height();
                }
            }
        }
    }
    return qMax(0.0, gross - deductions);
}

qreal WallItem::netVolume() const
{
    return netSurfaceArea() * (thicknessInMm() / 1000.0);
}

QJsonObject WallItem::toJson() const
{
    QJsonObject json = BaseEditorItem::toJson();

    json["p1_x"] = m_line.p1().x();
    json["p1_y"] = m_line.p1().y();
    json["p2_x"] = m_line.p2().x();
    json["p2_y"] = m_line.p2().y();
    json["thickness"] = m_thickness;
    json["alignment"] = m_alignment;

    return json;
}

void WallItem::fromJson(const QJsonObject &json)
{
    BaseEditorItem::fromJson(json);

    m_line.setP1(QPointF(json["p1_x"].toDouble(), json["p1_y"].toDouble()));
    m_line.setP2(QPointF(json["p2_x"].toDouble(), json["p2_y"].toDouble()));

    m_thickness = json["thickness"].toDouble();
    m_alignment = json["alignment"].toInt();

    updatePolygon();
    update();
}
