#include "roofitem.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QGraphicsSceneMouseEvent>
#include <QtMath>
#include <QPainterPathStroker>
#include <QJsonObject>
#include <QJsonArray>

RoofItem::RoofItem(QGraphicsItem *parent)
    : BaseEditorItem(parent), m_isDrawing(true), m_dragIndex(-1),
    m_roofType(Flat), m_overhang(0.5), m_angle(0.0)
{
    setZValue(5.0);
    setHeight(0.15);
}

QRectF RoofItem::boundingRect() const
{
    if (m_polygon.isEmpty()) return QRectF();
    qreal padding = m_overhang * 100.0 + 15.0;
    return m_polygon.boundingRect().adjusted(-padding, -padding, padding, padding);
}

QPainterPath RoofItem::shape() const
{
    QPainterPath path;
    if (m_polygon.isEmpty()) return path;

    path.addPolygon(m_polygon);

    qreal expand = m_overhang * 200.0;
    QPainterPathStroker stroker;
    stroker.setWidth(expand > 0 ? expand + 15 : 15);
    stroker.setJoinStyle(Qt::MiterJoin);

    QPainterPath finalPath = stroker.createStroke(path);
    finalPath.addPath(path);

    for (const QPointF &pt : m_polygon) {
        finalPath.addEllipse(pt, 15.0, 15.0);
    }

    return finalPath;
}

void RoofItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (m_polygon.isEmpty()) return;

    if (m_isDrawing) {
        painter->setBrush(QColor(150, 100, 50, 120));
        painter->setPen(QPen(QColor(100, 50, 20), 2, Qt::DashLine));
        painter->drawPolygon(m_polygon);

        painter->setBrush(Qt::white);
        painter->setPen(QPen(Qt::black, 1));
        for (const QPointF &pt : m_polygon) {
            painter->drawEllipse(pt, 3, 3);
        }
    } else {
        painter->setBrush(QColor(180, 90, 60, 240));

        if (m_overhang > 0) {
            QPen overhangPen(QColor(180, 90, 60, 240), m_overhang * 200.0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
            painter->setPen(overhangPen);
        } else {
            painter->setPen(QPen(QColor(80, 40, 20), 2, Qt::SolidLine));
        }
        painter->drawPolygon(m_polygon);

        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(80, 40, 20), 2, Qt::DashLine));
        painter->drawPolygon(m_polygon);

        if (m_roofType == Gable || m_roofType == Hip) {
            painter->save();

            QPainterPath clipPath;
            clipPath.addPolygon(m_polygon);
            painter->setClipPath(clipPath);

            painter->setPen(QPen(QColor(80, 40, 20), 1, Qt::SolidLine));
            QRectF rect = m_polygon.boundingRect();

            if (m_roofType == Gable) {
                if (rect.width() > rect.height()) {
                    painter->drawLine(QPointF(rect.left(), rect.center().y()),
                                      QPointF(rect.right(), rect.center().y()));
                } else {
                    painter->drawLine(QPointF(rect.center().x(), rect.top()),
                                      QPointF(rect.center().x(), rect.bottom()));
                }
            }
            else if (m_roofType == Hip) {
                painter->drawLine(rect.topLeft(), rect.bottomRight());
                painter->drawLine(rect.topRight(), rect.bottomLeft());
            }

            painter->restore();
        }

        if (isSelected()) {
            painter->setBrush(Qt::white);
            painter->setPen(QPen(QColor(21, 101, 192), 2));

            for (const QPointF &pt : m_polygon) {
                painter->drawRect(QRectF(pt.x() - 4, pt.y() - 4, 8, 8));
            }
        }
    }
}

int RoofItem::roofType() const { return m_roofType; }

void RoofItem::setRoofType(int type)
{
    if (m_roofType == type) return;
    prepareGeometryChange();
    m_roofType = type;
    update();
    emit itemChanged();
}

qreal RoofItem::overhang() const { return m_overhang; }

void RoofItem::setOverhang(qreal meters)
{
    if (qAbs(m_overhang - meters) < 1e-5) return;
    prepareGeometryChange();
    m_overhang = meters;
    update();
    emit itemChanged();
}

qreal RoofItem::angle() const { return m_angle; }

void RoofItem::setAngle(qreal degrees)
{
    if (degrees > 85.0) degrees = 85.0;
    if (degrees < 0.0) degrees = 0.0;
    if (qAbs(m_angle - degrees) < 1e-5) return;
    m_angle = degrees;
    update();
    emit itemChanged();
}

qreal RoofItem::area() const
{
    if (m_polygon.size() < 3) return 0.0;

    qreal totalArea = 0;
    qreal perimeter = 0;
    int n = m_polygon.size();
    for (int i = 0; i < n; ++i) {
        QPointF p1 = m_polygon[i];
        QPointF p2 = m_polygon[(i + 1) % n];
        totalArea += (p1.x() * p2.y() - p2.x() * p1.y());
        perimeter += QLineF(p1, p2).length();
    }

    qreal baseAreaMeters = qAbs(totalArea) / 2.0 / 10000.0;
    qreal perimeterMeters = perimeter / 100.0;

    qreal expandedAreaMeters = baseAreaMeters + (perimeterMeters * m_overhang) + (m_overhang * m_overhang * 4.0);

    if (m_roofType == Flat) {
        return expandedAreaMeters;
    } else {
        qreal angleRad = qDegreesToRadians(m_angle);
        if (qCos(angleRad) < 0.01) return expandedAreaMeters;
        return expandedAreaMeters / qCos(angleRad);
    }
}

void RoofItem::addPoint(const QPointF &scenePos)
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

void RoofItem::setDynamicPoint(const QPointF &scenePos)
{
    if (m_polygon.isEmpty() || !m_isDrawing) return;
    prepareGeometryChange();
    m_polygon.last() = mapFromScene(scenePos);
    update();
}

void RoofItem::finishDrawing()
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

void RoofItem::resumeDrawing(const QPointF &cursorScenePos)
{
    if (m_isDrawing) return;
    prepareGeometryChange();
    m_isDrawing = true;
    m_polygon.append(mapFromScene(cursorScenePos));
    update();
}

bool RoofItem::isDrawing() const { return m_isDrawing; }

int RoofItem::polygonSize() const { return m_polygon.size(); }

int RoofItem::vertexAt(const QPointF &localPos) const
{
    for (int i = 0; i < m_polygon.size(); ++i) {
        if (QLineF(localPos, m_polygon[i]).length() <= 15.0) return i;
    }
    return -1;
}

void RoofItem::removePoint(int index)
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

void RoofItem::removeLastPoint()
{
    if (m_isDrawing && m_polygon.size() > 1) {
        prepareGeometryChange();
        m_polygon.removeAt(m_polygon.size() - 2);
        update();
    }
}

QVariant RoofItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && !m_isDrawing) {
        return snapPosition(value.toPointF());
    }
    return BaseEditorItem::itemChange(change, value);
}

void RoofItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
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

void RoofItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
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

void RoofItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
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

qreal RoofItem::volume() const
{
    return area() * height();
}

QJsonObject RoofItem::toJson() const
{
    QJsonObject json = BaseEditorItem::toJson();

    QJsonArray polyArray;
    for (const QPointF &pt : m_polygon) {
        QJsonObject ptObj;
        ptObj["x"] = pt.x();
        ptObj["y"] = pt.y();
        polyArray.append(ptObj);
    }
    json["polygon"] = polyArray;

    json["roof_type"] = m_roofType;
    json["overhang"] = m_overhang;
    json["angle"] = m_angle;

    return json;
}

void RoofItem::fromJson(const QJsonObject &json)
{
    BaseEditorItem::fromJson(json);

    m_polygon.clear();
    QJsonArray polyArray = json["polygon"].toArray();
    for (const QJsonValue &val : polyArray) {
        QJsonObject ptObj = val.toObject();
        m_polygon.append(QPointF(ptObj["x"].toDouble(), ptObj["y"].toDouble()));
    }

    m_roofType = json["roof_type"].toInt();
    m_overhang = json["overhang"].toDouble();
    m_angle = json["angle"].toDouble();

    m_isDrawing = false;
    update();
}
