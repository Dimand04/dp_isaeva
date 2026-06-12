#ifndef WALLITEM_H
#define WALLITEM_H

#include "baseeditoritem.h"
#include <QLineF>
#include <QPolygonF>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QSet>

enum WallAlignment {
    AlignCenter = 0,
    AlignLeft = 1,
    AlignRight = 2
};

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
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setLengthInMeters(qreal lengthMeters);
    void setThicknessInMm(qreal thicknessMm);
    qreal lengthInMeters() const;
    qreal thicknessInMm() const;

    void setAngleInDegrees(qreal angleDeg);
    qreal angleInDegrees() const;

    void setAlignment(int alignment);
    int alignment() const;

    void updatePolygon();
    qreal calculateExactArea() const;

    qreal netArea() const;

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QLineF m_line;
    qreal m_thickness;
    QPolygonF m_polygon;
    bool m_isUpdating;
    WallInteractionState m_state;
    int m_alignment = 0;

    QSet<WallItem*> m_connectedWalls;

    QRectF startHandle() const;
    QRectF endHandle() const;
};

#endif
