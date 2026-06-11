#ifndef FOUNDATIONBLOCKITEM_H
#define FOUNDATIONBLOCKITEM_H

#include "baseeditoritem.h"
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

enum InteractionState {
    StateNone,
    StateResize,
    StateRotate
};

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

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    qreal m_width;
    qreal m_height;

    InteractionState m_state;

    QRectF resizeHandle() const;
    QRectF rotateHandle() const;
};

#endif
