#ifndef ROOFITEM_H
#define ROOFITEM_H

#include "baseeditoritem.h"
#include <QPolygonF>
#include <QPainterPath>

class RoofItem : public BaseEditorItem
{
    Q_OBJECT
public:
    explicit RoofItem(QGraphicsItem *parent = nullptr);

    enum RoofType {
        Flat = 0,
        Gable = 1,
        Hip = 2
    };

    void addPoint(const QPointF &scenePos);
    void setDynamicPoint(const QPointF &scenePos);
    void finishDrawing();
    void resumeDrawing(const QPointF &cursorScenePos);
    bool isDrawing() const;
    int polygonSize() const;
    void removePoint(int index);
    void removeLastPoint();

    int roofType() const;
    void setRoofType(int type);

    qreal overhang() const;
    void setOverhang(qreal meters);

    qreal angle() const;
    void setAngle(qreal degrees);

    qreal area() const;

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    int vertexAt(const QPointF &localPos) const;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:

    QPolygonF m_polygon;
    bool m_isDrawing;
    int m_dragIndex;

    int m_roofType;
    qreal m_overhang;
    qreal m_angle;
};

#endif // ROOFITEM_H
