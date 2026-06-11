#ifndef BASEEDITORITEM_H
#define BASEEDITORITEM_H

#include <QGraphicsObject>
#include <QPainter>
#include <QUuid>

class BaseEditorItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit BaseEditorItem(QGraphicsItem *parent = nullptr);
    virtual ~BaseEditorItem();

    QString getId() const { return m_id; }

    virtual QRectF boundingRect() const override = 0;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override = 0;

signals:
    void itemChanged();

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

private:
    QString m_id;
};

#endif
