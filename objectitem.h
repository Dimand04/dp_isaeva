#ifndef OBJECTITEM_H
#define OBJECTITEM_H

#include "baseeditoritem.h"
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

class ObjectItem : public BaseEditorItem
{
    Q_OBJECT
public:
    enum ObjectType {
        TypeBed = 0,
        TypeSofa,
        TypeTable,
        TypePlumbing,
        TypeWardrobe,
        TypeCustom
    };

    enum InteractionState {
        StateNone,
        StateResize,
        StateRotate
    };

    explicit ObjectItem(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setWidthInMeters(qreal widthMeters);
    void setLengthInMeters(qreal lengthMeters);
    void setObjectType(int type);

    qreal widthInMeters() const;
    qreal lengthInMeters() const;
    int objectType() const;

    qreal area() const;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

protected:
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    qreal m_width;
    qreal m_length;
    int m_objectType;
    InteractionState m_state;

    QRectF resizeHandle() const;
    QRectF rotateHandle() const;
    QString getTypeName() const;
    QColor getTypeColor() const;
};

#endif // OBJECTITEM_H
