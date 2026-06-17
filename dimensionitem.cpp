#include "dimensionitem.h"
#include <QPainter>
#include <QPen>
#include <QLineF>
#include <QGraphicsSceneMouseEvent>
#include <QtMath>
#include <QJsonObject>

DimensionItem::DimensionItem(QGraphicsItem *parent)
    : BaseEditorItem(parent), m_isDrawing(true), m_dragIndex(-1)
{
    setZValue(2.0);
    m_startPoint = QPointF(0, 0);
    m_endPoint = QPointF(0, 0);
}

void DimensionItem::setStartPoint(const QPointF &pt)
{
    prepareGeometryChange();
    setPos(pt);
    m_startPoint = QPointF(0, 0);
    m_endPoint = QPointF(0, 0);
    update();
}

void DimensionItem::setEndPoint(const QPointF &pt)
{
    prepareGeometryChange();
    m_endPoint = mapFromScene(pt);
    update();
}

void DimensionItem::finishDrawing()
{
    prepareGeometryChange();
    m_isDrawing = false;
    update();
    emit itemChanged();
}

bool DimensionItem::isDrawing() const
{
    return m_isDrawing;
}

qreal DimensionItem::lengthInMeters() const
{
    return QLineF(m_startPoint, m_endPoint).length() / 100.0;
}

int DimensionItem::textSide() const { return m_textSide; }

void DimensionItem::setTextSide(int side)
{
    if (m_textSide == side) return;
    prepareGeometryChange();
    m_textSide = side;
    update();
    emit itemChanged();
}

QRectF DimensionItem::boundingRect() const
{
    return shape().boundingRect().adjusted(-30, -30, 30, 30);
}

QPainterPath DimensionItem::shape() const
{
    QPainterPath path;
    path.moveTo(m_startPoint);
    path.lineTo(m_endPoint);

    QPainterPathStroker stroker;
    stroker.setWidth(15);
    QPainterPath finalPath = stroker.createStroke(path);

    finalPath.addPath(path);

    finalPath.addEllipse(m_startPoint, 12.0, 12.0);
    finalPath.addEllipse(m_endPoint, 12.0, 12.0);

    return finalPath;
}

void DimensionItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QLineF line(m_startPoint, m_endPoint);
    if (line.length() < 1.0) return;

    QColor color = isSelected() ? QColor(21, 101, 192) : QColor(50, 50, 50);
    painter->setPen(QPen(color, 1, Qt::SolidLine));

    painter->drawLine(line);

    qreal angle = line.angle();

    QLineF tick1 = QLineF::fromPolar(6, angle + 90).translated(m_startPoint);
    QLineF tick2 = QLineF::fromPolar(6, angle - 90).translated(m_startPoint);
    painter->drawLine(tick1.p2(), tick2.p2());

    QLineF tick3 = QLineF::fromPolar(6, angle + 90).translated(m_endPoint);
    QLineF tick4 = QLineF::fromPolar(6, angle - 90).translated(m_endPoint);
    painter->drawLine(tick3.p2(), tick4.p2());

    painter->save();

    QPointF center = line.center();
    painter->translate(center);

    qreal textAngle = -angle;
    while (textAngle <= -180.0) textAngle += 360.0;
    while (textAngle > 180.0) textAngle -= 360.0;

    if (textAngle < -90.0 || textAngle > 90.0) {
        textAngle += 180.0;
    }
    painter->rotate(textAngle);

    qreal yOffset = 0.0;
    if (m_textSide == 0) {
        yOffset = -12.0;
    } else {
        yOffset = 12.0;
    }

    QString text = QString::number(line.length() * 10.0, 'f', 0);

    QFont font = painter->font();
    font.setPointSize(9);
    painter->setFont(font);

    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(text);

    QRectF bgRect(-textRect.width() / 2.0 - 4, yOffset - textRect.height() / 2.0, textRect.width() + 8, textRect.height());

    painter->setBrush(Qt::white);
    painter->setPen(Qt::NoPen);
    painter->drawRect(bgRect);

    painter->setPen(color);
    painter->drawText(bgRect, Qt::AlignCenter, text);

    painter->restore();

    if (isSelected() && !m_isDrawing) {
        painter->setBrush(Qt::white);
        painter->setPen(QPen(QColor(21, 101, 192), 2));
        painter->drawRect(QRectF(m_startPoint.x() - 5, m_startPoint.y() - 5, 10, 10));
        painter->drawRect(QRectF(m_endPoint.x() - 5, m_endPoint.y() - 5, 10, 10));
    }
}

int DimensionItem::vertexAt(const QPointF &localPos) const
{
    if (QLineF(localPos, m_startPoint).length() <= 12.0) return 0;
    if (QLineF(localPos, m_endPoint).length() <= 12.0) return 1;
    return -1;
}

QVariant DimensionItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && !m_isDrawing) {
        return snapPosition(value.toPointF());
    }
    return BaseEditorItem::itemChange(change, value);
}

void DimensionItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
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

void DimensionItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragIndex != -1) {
        QPointF scenePos = event->scenePos();
        QPointF snappedScene = snapPosition(scenePos);

        prepareGeometryChange();
        if (m_dragIndex == 0) {
            QPointF oldSceneEnd = mapToScene(m_endPoint);
            setPos(snappedScene);
            m_endPoint = mapFromScene(oldSceneEnd);
        } else if (m_dragIndex == 1) {
            m_endPoint = mapFromScene(snappedScene);
        }
        update();
        emit itemChanged();
        return;
    }
    BaseEditorItem::mouseMoveEvent(event);
}

void DimensionItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_dragIndex != -1) {
        m_dragIndex = -1;
        event->accept();
        return;
    }
    BaseEditorItem::mouseReleaseEvent(event);
}

QJsonObject DimensionItem::toJson() const
{
    QJsonObject json = BaseEditorItem::toJson();
    json["start_x"] = m_startPoint.x();
    json["start_y"] = m_startPoint.y();
    json["end_x"] = m_endPoint.x();
    json["end_y"] = m_endPoint.y();
    json["text_side"] = m_textSide;
    return json;
}

void DimensionItem::fromJson(const QJsonObject &json)
{
    BaseEditorItem::fromJson(json);
    m_startPoint = QPointF(json["start_x"].toDouble(), json["start_y"].toDouble());
    m_endPoint = QPointF(json["end_x"].toDouble(), json["end_y"].toDouble());
    m_textSide = json["text_side"].toInt();

    m_isDrawing = false;
    update();
}
