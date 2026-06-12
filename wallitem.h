#ifndef WALLITEM_H
#define WALLITEM_H

#include "baseeditoritem.h"
#include <QLineF>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

enum WallInteractionState {
    WallStateNone,
    WallStateMoveStart,
    WallStateMoveEnd
};

class WallItem : public BaseEditorItem
{
    Q_OBJECT
public:
    explicit WallItem(const QPointF &startPoint, QGraphicsItem *parent = nullptr);

    void setEndPoint(const QPointF &endPoint);
    QLineF line() const { return m_line; }

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setLengthInMeters(qreal lengthMeters);
    void setThicknessInMm(qreal thicknessMm);
    qreal lengthInMeters() const;
    qreal thicknessInMm() const;

    void setAngleInDegrees(qreal angleDeg);
    qreal angleInDegrees() const;

    QPainterPath shape() const override;

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QLineF m_line;
    qreal m_thickness;

    QRectF startHandle() const;
    QRectF endHandle() const;
    WallInteractionState m_state;
};

#endif
