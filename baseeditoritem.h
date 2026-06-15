#ifndef BASEEDITORITEM_H
#define BASEEDITORITEM_H

#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QVariant>

class BaseEditorItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit BaseEditorItem(QGraphicsItem *parent = nullptr);
    virtual ~BaseEditorItem() = default;
    void setName(const QString &name);
    QString name() const;

protected:
    virtual QPointF snapPosition(const QPointF &pos) const;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void itemChanged();

private:
    QString m_name;
};

#endif
