#include "textitem.h"
#include <QPainter>
#include <QFontMetrics>
#include <QLineF>
#include <QtMath>
#include <QCursor>
#include <QGraphicsSceneHoverEvent>

TextItem::TextItem(QGraphicsItem *parent)
    : BaseEditorItem(parent), m_textContent("Текст"), m_fontSize(14), m_textColor(Qt::black), m_dragIndex(-1), m_state(StateNone)
{
    setZValue(3.0);
    setAcceptHoverEvents(true);
}

QString TextItem::textContent() const { return m_textContent; }

void TextItem::setTextContent(const QString &text)
{
    if (m_textContent == text) return;
    prepareGeometryChange();
    m_textContent = text;
    update();
    emit itemChanged();
}

int TextItem::fontSize() const { return m_fontSize; }

void TextItem::setFontSize(int size)
{
    if (m_fontSize == size) return;
    prepareGeometryChange();
    m_fontSize = size;
    update();
    emit itemChanged();
}

QColor TextItem::textColor() const { return m_textColor; }

void TextItem::setTextColor(const QColor &color)
{
    if (m_textColor == color) return;
    m_textColor = color;
    update();
    emit itemChanged();
}

qreal TextItem::rotationAngle() const
{
    return rotation();
}

void TextItem::setRotationAngle(qreal angle)
{
    if (qAbs(rotation() - angle) < 1e-5) return;
    setRotation(angle);
    emit itemChanged();
}

QRectF TextItem::boundingRect() const
{
    QFont font("Arial", m_fontSize);
    QFontMetrics fm(font);
    QRect r = fm.boundingRect(m_textContent);

    QRectF centeredRect(-r.width() / 2.0, -r.height() / 2.0, r.width(), r.height());
    return centeredRect.adjusted(-25, -45, 25, 25);
}

QPainterPath TextItem::shape() const
{
    QPainterPath path;
    QFont font("Arial", m_fontSize);
    QFontMetrics fm(font);
    QRect r = fm.boundingRect(m_textContent);
    QRectF centeredRect(-r.width() / 2.0, -r.height() / 2.0, r.width(), r.height());

    path.addRect(centeredRect);

    if (isSelected()) {
        path.addEllipse(rotateHandle());
        path.addEllipse(centeredRect.topLeft(), 20.0, 20.0);
        path.addEllipse(centeredRect.topRight(), 20.0, 20.0);
        path.addEllipse(centeredRect.bottomLeft(), 20.0, 20.0);
        path.addEllipse(centeredRect.bottomRight(), 20.0, 20.0);
    }
    return path;
}

void TextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QFont font("Arial", m_fontSize);
    painter->setFont(font);
    painter->setPen(m_textColor);

    QFontMetrics fm(font);
    QRect r = fm.boundingRect(m_textContent);
    QRectF centeredRect(-r.width() / 2.0, -r.height() / 2.0, r.width(), r.height());

    painter->drawText(centeredRect, Qt::AlignCenter, m_textContent);

    if (isSelected()) {
        painter->setPen(QPen(QColor(21, 101, 192), 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(centeredRect);

        painter->setBrush(Qt::white);
        painter->setPen(QPen(QColor(21, 101, 192), 2));

        painter->drawRect(QRectF(centeredRect.topLeft().x() - 6, centeredRect.topLeft().y() - 6, 12, 12));
        painter->drawRect(QRectF(centeredRect.topRight().x() - 6, centeredRect.topRight().y() - 6, 12, 12));
        painter->drawRect(QRectF(centeredRect.bottomLeft().x() - 6, centeredRect.bottomLeft().y() - 6, 12, 12));
        painter->drawRect(QRectF(centeredRect.bottomRight().x() - 6, centeredRect.bottomRight().y() - 6, 12, 12));

        painter->drawLine(0, -centeredRect.height() / 2.0, 0, -centeredRect.height() / 2.0 - 20);
        painter->drawEllipse(rotateHandle());
    }
}

int TextItem::vertexAt(const QPointF &localPos) const
{
    QFont font("Arial", m_fontSize);
    QFontMetrics fm(font);
    QRect r = fm.boundingRect(m_textContent);
    QRectF centeredRect(-r.width() / 2.0, -r.height() / 2.0, r.width(), r.height());

    if (QLineF(localPos, centeredRect.topLeft()).length() <= 20.0) return 0;
    if (QLineF(localPos, centeredRect.topRight()).length() <= 20.0) return 1;
    if (QLineF(localPos, centeredRect.bottomLeft()).length() <= 20.0) return 2;
    if (QLineF(localPos, centeredRect.bottomRight()).length() <= 20.0) return 3;
    return -1;
}

QVariant TextItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        return snapPosition(value.toPointF());
    }
    return BaseEditorItem::itemChange(change, value);
}

void TextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelected()) {
        if (rotateHandle().contains(event->pos())) {
            m_state = StateRotate;
            setCursor(Qt::ClosedHandCursor);
            event->accept();
            return;
        }

        int idx = vertexAt(event->pos());
        if (idx != -1) {
            m_state = StateResize;
            m_dragIndex = idx;
            m_startDragPos = event->scenePos();
            m_startFontSize = m_fontSize;
            m_startCenter = mapToScene(0, 0);
            event->accept();
            return;
        }
    }
    m_state = StateNone;
    BaseEditorItem::mousePressEvent(event);
}

void TextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_state == StateResize && m_dragIndex != -1) {
        QPointF currentPos = event->scenePos();
        qreal oldDist = QLineF(m_startCenter, m_startDragPos).length();
        qreal newDist = QLineF(m_startCenter, currentPos).length();

        if (oldDist > 0) {
            int newSize = qMax(4, qRound(m_startFontSize * (newDist / oldDist)));
            setFontSize(newSize);
        }
        return;
    } else if (m_state == StateRotate) {
        QPointF centerInScene = mapToScene(0, 0);
        QPointF dir = event->scenePos() - centerInScene;

        qreal angleRad = qAtan2(dir.y(), dir.x());
        qreal angleDeg = qRadiansToDegrees(angleRad) + 90.0;

        while (angleDeg < 0) angleDeg += 360;
        while (angleDeg >= 360) angleDeg -= 360;

        bool isSnapped = false;
        if (!(event->modifiers() & Qt::ShiftModifier)) {
            QList<qreal> snapAngles = {0.0, 90.0, 180.0, 270.0, 360.0};
            for (qreal target : snapAngles) {
                if (qAbs(angleDeg - target) <= 5.0) {
                    angleDeg = (target == 360.0) ? 0.0 : target;
                    isSnapped = true;
                    break;
                }
            }
        }

        if (!isSnapped && (event->modifiers() & Qt::ShiftModifier)) {
            angleDeg = qRound(angleDeg / 15.0) * 15.0;
        }

        setRotationAngle(angleDeg);
        return;
    }
    BaseEditorItem::mouseMoveEvent(event);
}

void TextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_state == StateRotate) {
        setCursor(Qt::OpenHandCursor);
    }
    if (m_state != StateNone) {
        m_state = StateNone;
        m_dragIndex = -1;
        event->accept();
        return;
    }
    BaseEditorItem::mouseReleaseEvent(event);
}

QRectF TextItem::rotateHandle() const
{
    QFont font("Arial", m_fontSize);
    QFontMetrics fm(font);
    QRect r = fm.boundingRect(m_textContent);
    return QRectF(-5, -r.height() / 2.0 - 25, 10, 10);
}

void TextItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (isSelected()) {
        if (vertexAt(event->pos()) != -1) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (rotateHandle().contains(event->pos())) {
            setCursor(Qt::OpenHandCursor);
        } else {
            setCursor(Qt::SizeAllCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
    BaseEditorItem::hoverMoveEvent(event);
}
