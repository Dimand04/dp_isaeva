#include "wallitem.h"
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QtMath>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QPainterPathStroker>

WallItem::WallItem(const QPointF &startPoint, QGraphicsItem *parent)
    : BaseEditorItem(parent), m_thickness(20.0), m_state(WallStateNone)
{
    setAcceptHoverEvents(true);
    m_line.setP1(QPointF(0, 0));
    m_line.setP2(QPointF(0, 0));
}

void WallItem::setEndPoint(const QPointF &endPoint)
{
    prepareGeometryChange();
    m_line.setP2(mapFromScene(endPoint));
}

void WallItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen;
    pen.setWidthF(m_thickness);
    pen.setCapStyle(Qt::FlatCap);

    if (isSelected()) {
        pen.setColor(QColor(21, 101, 192));
    } else {
        pen.setColor(QColor(50, 50, 50));
    }

    painter->setPen(pen);
    painter->drawLine(m_line);

    painter->setPen(QPen(Qt::white, 1, Qt::DashLine));
    painter->drawLine(m_line);

    if (isSelected()) {
        painter->setBrush(Qt::white);
        painter->setPen(QPen(QColor(21, 101, 192), 1));
        painter->drawRect(startHandle());
        painter->drawRect(endHandle());
    }
}

void WallItem::setLengthInMeters(qreal lengthMeters)
{
    prepareGeometryChange();
    qreal lengthPx = lengthMeters * 100.0;

    qreal angleRad = qDegreesToRadians(m_line.angle());
    qreal newX = m_line.p1().x() + lengthPx * qCos(angleRad);
    qreal newY = m_line.p1().y() - lengthPx * qSin(angleRad);

    m_line.setP2(QPointF(newX, newY));
    update();
}

void WallItem::setThicknessInMm(qreal thicknessMm)
{
    prepareGeometryChange();
    m_thickness = thicknessMm * 0.1;
    update();
}

qreal WallItem::lengthInMeters() const
{
    return m_line.length() / 100.0;
}

qreal WallItem::thicknessInMm() const
{
    return m_thickness / 0.1;
}

QRectF WallItem::startHandle() const {
    return QRectF(-5, -5, 10, 10);
}

QRectF WallItem::endHandle() const {
    return QRectF(m_line.p2().x() - 5, m_line.p2().y() - 5, 10, 10);
}

QRectF WallItem::boundingRect() const
{
    qreal extra = m_thickness / 2.0 + 15.0;
    return QRectF(m_line.p1(), m_line.p2()).normalized().adjusted(-extra, -extra, extra, extra);
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
    int gridSize = 20;

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

void WallItem::setAngleInDegrees(qreal angleDeg)
{
    prepareGeometryChange();
    m_line.setAngle(angleDeg);
    update();
}

qreal WallItem::angleInDegrees() const
{
    qreal angle = m_line.angle();
    if (angle < 0) angle += 360;
    return angle;
}

QPainterPath WallItem::shape() const
{
    QPainterPath path;
    path.moveTo(m_line.p1());
    path.lineTo(m_line.p2());

    QPainterPathStroker stroker;
    stroker.setWidth(m_thickness + 20);
    return stroker.createStroke(path);
}
