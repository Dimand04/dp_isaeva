#ifndef DOORITEM_H
#define DOORITEM_H

#include "baseeditoritem.h"
#include <QGraphicsSceneMouseEvent>

class WallItem;

class DoorItem : public BaseEditorItem
{
    Q_OBJECT
public:
    explicit DoorItem(qreal width, WallItem *hostWall, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setWidthInMeters(qreal widthMeters);
    void setSwingType(int type);
    qreal widthInMeters() const;
    int swingType() const;

    qreal distanceFromStart() const;
    void setDistanceFromStart(qreal distanceMeters);

    void snapToPos(const QPointF &scenePt);

    WallItem* hostWall() const { return m_hostWall; }

    qreal area() const;

public slots:
    void updateGeometryToWall();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    qreal m_width;
    qreal m_depth;
    int m_swingType;
    qreal m_distance;

    WallItem *m_hostWall;
    bool m_isDragging;
};

#endif
