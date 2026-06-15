#ifndef WINDOWITEM_H
#define WINDOWITEM_H

#include "baseeditoritem.h"
#include <QPainterPath>

class WallItem;

class WindowItem : public BaseEditorItem
{
    Q_OBJECT
public:
    explicit WindowItem(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setHostWall(WallItem *wall);
    WallItem* hostWall() const;
    void updateGeometryToWall();

    void setWidthInMeters(qreal w);
    qreal widthInMeters() const;

    void setElevation(qreal e);
    qreal elevation() const;

    void setProfileType(int type);
    int profileType() const;

    void setDistanceFromStart(qreal distanceMeters);
    qreal distanceFromStart() const;

    qreal area() const;

protected:
    QPointF snapPosition(const QPointF &pos) const override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    WallItem *m_hostWall;
    qreal m_width;
    qreal m_depth;
    qreal m_elevation;
    int m_profileType;
    qreal m_distance;
};

#endif // WINDOWITEM_H
