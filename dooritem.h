#ifndef DOORITEM_H
#define DOORITEM_H

#include "baseeditoritem.h"
#include <QPainterPath>

class WallItem;

class DoorItem : public BaseEditorItem
{
    Q_OBJECT
public:
    explicit DoorItem(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setHostWall(WallItem *wall);
    WallItem* hostWall() const;
    void updateGeometryToWall();

    void setWidthInMeters(qreal w);
    qreal widthInMeters() const;

    void setSwingType(int type);
    int swingType() const;

    void setDistanceFromStart(qreal distanceMeters);
    qreal distanceFromStart() const;

    qreal area() const;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

protected:
    QPointF snapPosition(const QPointF &pos) const override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    WallItem *m_hostWall;
    qreal m_width;
    qreal m_depth;
    int m_swingType;
    qreal m_distance;
};

#endif // DOORITEM_H
