#ifndef WINDOWITEM_H
#define WINDOWITEM_H

#include "baseeditoritem.h"
#include <QGraphicsSceneMouseEvent>

class WallItem;

class WindowItem : public BaseEditorItem
{
    Q_OBJECT
public:
    explicit WindowItem(qreal width, WallItem *hostWall, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setWidthInMeters(qreal widthMeters);
    void setElevation(qreal elevation);
    void setProfileType(int index);

    qreal widthInMeters() const { return m_width / 100.0; }
    qreal elevation() const { return m_elevation; }
    int profileType() const { return m_profileType; }

    qreal distanceFromStart() const;

    void snapToPos(const QPointF &scenePt);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    qreal m_width;
    qreal m_depth;
    qreal m_elevation = 0.8;
    int m_profileType = 0;

    WallItem *m_hostWall;
    bool m_isDragging;
};

#endif
