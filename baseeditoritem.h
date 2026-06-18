#ifndef BASEEDITORITEM_H
#define BASEEDITORITEM_H

#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QVariant>
#include <QJsonObject>

class BaseEditorItem : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit BaseEditorItem(QGraphicsItem *parent = nullptr);
    virtual ~BaseEditorItem() = default;

    void setName(const QString &name);
    QString name() const;

    void setLevelId(int levelId);
    int levelId() const;

    void setLayerName(const QString &layerName);
    QString layerName() const;

    virtual void setHeight(qreal height);
    virtual qreal height() const;

    virtual int type() const override { return UserType + 1; }
    virtual QJsonObject toJson() const;
    virtual void fromJson(const QJsonObject &json);

    static BaseEditorItem* createFromJson(const QJsonObject &json);

    void setMaterialId(int id) {
        if (m_materialId != id) {
            m_materialId = id;
            emit itemChanged();
        }
    }
    int materialId() const { return m_materialId; }

protected:
    virtual QPointF snapPosition(const QPointF &pos) const;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void itemChanged();

private:
    QString m_name;
    int m_levelId;
    QString m_layerName;
    qreal m_height;
    int m_materialId = -1;
};

#endif
