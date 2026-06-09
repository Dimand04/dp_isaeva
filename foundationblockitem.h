#ifndef FOUNDATIONBLOCKITEM_H
#define FOUNDATIONBLOCKITEM_H

#include "baseeditoritem.h"

class FoundationBlockItem : public BaseEditorItem
{
    Q_OBJECT
public:
    explicit FoundationBlockItem(qreal width, qreal height, QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void setWidthInMeters(qreal widthMeters);
    void setHeightInMeters(qreal heightMeters);
    qreal widthInMeters() const;
    qreal heightInMeters() const;

private:
    qreal m_width;
    qreal m_height;
};

#endif
