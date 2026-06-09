#include "foundationblockitem.h"
#include <QPen>
#include <QBrush>

FoundationBlockItem::FoundationBlockItem(qreal width, qreal height, QGraphicsItem *parent)
    : BaseEditorItem(parent), m_width(width), m_height(height)
{
}

QRectF FoundationBlockItem::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height).adjusted(-2, -2, 2, 2);
}

void FoundationBlockItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QRectF rect(0, 0, m_width, m_height);

    painter->setBrush(QColor(200, 200, 200, 150));

    if (isSelected()) {
        painter->setPen(QPen(QColor(21, 101, 192), 2, Qt::DashLine));
    } else {
        painter->setPen(QPen(QColor(100, 100, 100), 1, Qt::SolidLine));
    }

    painter->drawRect(rect);
}

void FoundationBlockItem::setWidthInMeters(qreal widthMeters)
{
    prepareGeometryChange();
    m_width = widthMeters * 100.0;
    update();
}

void FoundationBlockItem::setHeightInMeters(qreal heightMeters)
{
    prepareGeometryChange();
    m_height = heightMeters * 100.0;
    update();
}

qreal FoundationBlockItem::widthInMeters() const
{
    return m_width / 100.0;
}

qreal FoundationBlockItem::heightInMeters() const
{
    return m_height / 100.0;
}
