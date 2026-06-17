#ifndef NODEITEM_H
#define NODEITEM_H

#include "baseeditoritem.h"
#include <QPainterPath>

class NodeItem : public BaseEditorItem
{
    Q_OBJECT
public:
    explicit NodeItem(const QPointF &pos, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setSide1InMeters(qreal w);
    void setSide2InMeters(qreal h);
    void setRounded(int isRounded);
    void setRotationAngle(qreal angle);

    qreal side1InMeters() const;
    qreal side2InMeters() const;
    int isRounded() const;
    qreal rotationAngle() const;

    qreal area() const;

    qreal maxAttachedWallHeight() const;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    qreal m_width;
    qreal m_height;
    int m_isRounded;
    bool m_isDragging;

    QPainterPath currentPath() const;
};

#endif // NODEITEM_H
