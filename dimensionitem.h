#ifndef DIMENSIONITEM_H
#define DIMENSIONITEM_H

#include "baseeditoritem.h"
#include <QPointF>

class DimensionItem : public BaseEditorItem
{
    Q_OBJECT
public:
    explicit DimensionItem(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setStartPoint(const QPointF &pt);
    void setEndPoint(const QPointF &pt);
    void finishDrawing();
    bool isDrawing() const;

    qreal lengthInMeters() const;
    int vertexAt(const QPointF &localPos) const;

    int textSide() const;
    void setTextSide(int side);

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QPointF m_startPoint;
    QPointF m_endPoint;
    bool m_isDrawing;
    int m_dragIndex;
    int m_textSide = 0;
};

#endif // DIMENSIONITEM_H
