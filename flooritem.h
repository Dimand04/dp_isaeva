#ifndef FLOORITEM_H
#define FLOORITEM_H

#include "baseeditoritem.h"
#include <QPolygonF>

class FloorItem : public BaseEditorItem
{
    Q_OBJECT
public:
    explicit FloorItem(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void addPoint(const QPointF &scenePos);
    void setDynamicPoint(const QPointF &scenePos);

    void finishDrawing();
    void resumeDrawing(const QPointF &cursorScenePos);

    bool isDrawing() const;
    qreal area() const;

    int polygonSize() const;
    int vertexAt(const QPointF &localPos) const;
    void removePoint(int index);
    void removeLastPoint();

    qreal volume() const;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QPolygonF m_polygon;
    bool m_isDrawing;
    int m_dragIndex;
};

#endif // FLOORITEM_H
