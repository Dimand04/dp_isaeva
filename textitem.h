#ifndef TEXTITEM_H
#define TEXTITEM_H

#include "baseeditoritem.h"
#include <QFont>
#include <QColor>
#include <QGraphicsSceneMouseEvent>

class TextItem : public BaseEditorItem
{
    Q_OBJECT
public:
    explicit TextItem(QGraphicsItem *parent = nullptr);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    QString textContent() const;
    void setTextContent(const QString &text);

    int fontSize() const;
    void setFontSize(int size);

    QColor textColor() const;
    void setTextColor(const QColor &color);

    qreal rotationAngle() const;
    void setRotationAngle(qreal angle);

    int vertexAt(const QPointF &localPos) const;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject &json) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    QString m_textContent;
    int m_fontSize;
    QColor m_textColor;

    int m_dragIndex;
    qreal m_startFontSize;
    QPointF m_startDragPos;
    QPointF m_startCenter;

    enum State { StateNone, StateResize, StateRotate };
    State m_state;
    QRectF rotateHandle() const;
};

#endif
