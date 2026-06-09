#include "wallitem.h"
#include <QPen>

WallItem::WallItem(const QPointF &startPoint, QGraphicsItem *parent)
    : BaseEditorItem(parent), m_thickness(20.0)
{
    m_line.setP1(QPointF(0, 0));
    m_line.setP2(QPointF(0, 0));
}

void WallItem::setEndPoint(const QPointF &endPoint)
{
    prepareGeometryChange();
    m_line.setP2(mapFromScene(endPoint));
}

QRectF WallItem::boundingRect() const
{
    qreal extra = m_thickness / 2.0 + 2.0;
    return QRectF(m_line.p1(), m_line.p2()).normalized().adjusted(-extra, -extra, extra, extra);
}

void WallItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QPen pen;
    pen.setWidthF(m_thickness);
    pen.setCapStyle(Qt::FlatCap);

    if (isSelected()) {
        pen.setColor(QColor(21, 101, 192));
    } else {
        pen.setColor(QColor(50, 50, 50));
    }

    painter->setPen(pen);
    painter->drawLine(m_line);

    painter->setPen(QPen(Qt::white, 1, Qt::DashLine));
    painter->drawLine(m_line);
}

void WallItem::setLengthInMeters(qreal lengthMeters)
{
    prepareGeometryChange();
    qreal lengthPx = lengthMeters * 100.0;

    qreal angleRad = qDegreesToRadians(m_line.angle());
    qreal newX = m_line.p1().x() + lengthPx * qCos(angleRad);
    qreal newY = m_line.p1().y() - lengthPx * qSin(angleRad);

    m_line.setP2(QPointF(newX, newY));
    update();
}

void WallItem::setThicknessInMm(qreal thicknessMm)
{
    prepareGeometryChange();
    m_thickness = thicknessMm * 0.1;
    update();
}

qreal WallItem::lengthInMeters() const
{
    return m_line.length() / 100.0;
}

qreal WallItem::thicknessInMm() const
{
    return m_thickness / 0.1;
}
