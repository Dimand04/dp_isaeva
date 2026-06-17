#include "foundationblockitem.h"
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QtMath>
#include <QGraphicsView>
#include <QJsonObject>

FoundationBlockItem::FoundationBlockItem(qreal width, qreal length, QGraphicsItem *parent)
    : BaseEditorItem(parent), m_width(width), m_length(length), m_state(StateNone)
{
    setAcceptHoverEvents(true);
    setZValue(0.0);
    setTransformOriginPoint(m_width / 2, m_length / 2);

    setHeight(1.0);
}

QRectF FoundationBlockItem::resizeHandle() const {
    return QRectF(m_width - 10, m_length - 10, 10, 10);
}

QRectF FoundationBlockItem::rotateHandle() const {
    return QRectF(m_width / 2 - 5, -25, 10, 10);
}

QRectF FoundationBlockItem::boundingRect() const {
    return QRectF(0, -30, m_width, m_length + 30).adjusted(-5, -5, 5, 5);
}

void FoundationBlockItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QRectF rect(0, 0, m_width, m_length);

    painter->setBrush(QColor(200, 200, 200, 150));
    if (isSelected()) {
        painter->setPen(QPen(QColor(21, 101, 192), 2, Qt::DashLine));
    } else {
        painter->setPen(QPen(QColor(100, 100, 100), 1, Qt::SolidLine));
    }
    painter->drawRect(rect);

    if (isSelected()) {
        painter->setBrush(Qt::white);
        painter->setPen(QPen(QColor(21, 101, 192), 1));

        painter->drawRect(resizeHandle());

        painter->drawLine(m_width / 2, 0, m_width / 2, -20);
        painter->drawEllipse(rotateHandle());
    }
}

void FoundationBlockItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (isSelected()) {
        if (resizeHandle().contains(event->pos())) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (rotateHandle().contains(event->pos())) {
        } else {
            setCursor(Qt::SizeAllCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
    BaseEditorItem::hoverMoveEvent(event);
}

void FoundationBlockItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelected()) {
        if (resizeHandle().contains(event->pos())) {
            m_state = StateResize;
            return;
        } else if (rotateHandle().contains(event->pos())) {
            m_state = StateRotate;
            setCursor(Qt::ClosedHandCursor);
            return;
        }
    }
    m_state = StateNone;
    BaseEditorItem::mousePressEvent(event);
}

void FoundationBlockItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_state == StateResize) {
        prepareGeometryChange();

        int gridSize = 20;

        qreal rawWidth = qMax(20.0, event->pos().x());
        qreal rawHeight = qMax(20.0, event->pos().y());

        m_width = qRound(rawWidth / gridSize) * gridSize;
        m_length = qRound(rawHeight / gridSize) * gridSize;

        setTransformOriginPoint(m_width / 2, m_length / 2);

        scene()->update();
        emit itemChanged();

    } else if (m_state == StateRotate) {
        QPointF centerInScene = mapToScene(m_width / 2, m_length / 2);

        QPointF dir = event->scenePos() - centerInScene;

        qreal angleRad = qAtan2(dir.y(), dir.x());
        qreal angleDeg = qRadiansToDegrees(angleRad) + 90.0;

        while (angleDeg < 0) angleDeg += 360;
        while (angleDeg >= 360) angleDeg -= 360;

        bool isSnapped = false;

        if (!(event->modifiers() & Qt::ShiftModifier)) {
            QList<qreal> snapAngles = {0.0, 90.0, 180.0, 270.0, 360.0};
            for (qreal target : snapAngles) {
                if (qAbs(angleDeg - target) <= 5.0) {
                    angleDeg = (target == 360.0) ? 0.0 : target;
                    isSnapped = true;
                    break;
                }
            }
        }

        if (!isSnapped && (event->modifiers() & Qt::ShiftModifier)) {
            angleDeg = qRound(angleDeg / 15.0) * 15.0;
        }

        setRotation(angleDeg);
        emit itemChanged();

    } else {
        BaseEditorItem::mouseMoveEvent(event);
    }
}

void FoundationBlockItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_state == StateRotate) {
        setCursor(Qt::OpenHandCursor);
    }
    m_state = StateNone;
    BaseEditorItem::mouseReleaseEvent(event);
}

void FoundationBlockItem::setWidthInMeters(qreal widthMeters)
{
    if (qAbs(widthInMeters() - widthMeters) < 1e-5) return;

    prepareGeometryChange();
    m_width = widthMeters * 100.0;

    setTransformOriginPoint(m_width / 2.0, m_length / 2.0);

    update();
    emit itemChanged();
}

void FoundationBlockItem::setLengthInMeters(qreal lengthMeters)
{
    if (qAbs(lengthInMeters() - lengthMeters) < 1e-5) return;

    prepareGeometryChange();
    m_length = lengthMeters * 100.0;

    setTransformOriginPoint(m_width / 2.0, m_length / 2.0);

    update();
    emit itemChanged();
}

qreal FoundationBlockItem::widthInMeters() const
{
    return m_width / 100.0;
}

qreal FoundationBlockItem::lengthInMeters() const
{
    return m_length / 100.0;
}

qreal FoundationBlockItem::area() const
{
    return widthInMeters() * lengthInMeters();
}

qreal FoundationBlockItem::volume() const
{
    return area() * height();
}

QJsonObject FoundationBlockItem::toJson() const
{
    QJsonObject json = BaseEditorItem::toJson();

    json["width"] = m_width;
    json["length"] = m_length;

    return json;
}

void FoundationBlockItem::fromJson(const QJsonObject &json)
{
    BaseEditorItem::fromJson(json);

    m_width = json["width"].toDouble();
    m_length = json["length"].toDouble();

    setTransformOriginPoint(m_width / 2.0, m_length / 2.0);

    update();
}
