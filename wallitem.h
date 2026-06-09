#ifndef WALLITEM_H
#define WALLITEM_H

#include "baseeditoritem.h"
#include <QLineF>

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

private:
    QLineF m_line;
    qreal m_thickness;
};

#endif
