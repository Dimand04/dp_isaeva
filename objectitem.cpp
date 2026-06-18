#include "objectitem.h"
#include <QPainter>
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QtMath>
#include <QGraphicsScene>

ObjectItem::ObjectItem(QGraphicsItem *parent)
    : BaseEditorItem(parent), m_width(100.0), m_length(100.0), m_objectType(TypeTable), m_state(StateNone)
{
    setAcceptHoverEvents(true);
    setZValue(2.0);
    setTransformOriginPoint(m_width / 2.0, m_length / 2.0);
    setHeight(0.8);
}

QRectF ObjectItem::resizeHandle() const {
    return QRectF(m_width - 10, m_length - 10, 10, 10);
}

QRectF ObjectItem::rotateHandle() const {
    return QRectF(m_width / 2.0 - 6, -30, 12, 12);
}

QRectF ObjectItem::boundingRect() const {
    return QRectF(0, -35, m_width, m_length + 35).adjusted(-5, -5, 5, 5);
}

QString ObjectItem::getTypeName() const
{
    return m_materialName.isEmpty() ? "Объект" : m_materialName;
}

QColor ObjectItem::getTypeColor() const
{
    return QColor(200, 210, 225, 180);
}

void ObjectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    QRectF rect(0, 0, m_width, m_length);

    painter->setBrush(getTypeColor());
    if (isSelected()) {
        painter->setPen(QPen(QColor(21, 101, 192), 2, Qt::DashLine));
    } else {
        painter->setPen(QPen(QColor(80, 80, 80), 1, Qt::SolidLine));
    }
    painter->drawRect(rect);

    painter->setPen(Qt::black);
    QFont font = painter->font();
    font.setPointSize(8);
    painter->setFont(font);

    painter->drawText(rect, Qt::AlignCenter | Qt::TextWordWrap, getTypeName());

    painter->setPen(QPen(QColor(80, 80, 80), 2));
    painter->drawLine(QPointF(0, 0), QPointF(m_width, 0));

    if (isSelected()) {
        painter->setBrush(Qt::white);
        painter->setPen(QPen(QColor(21, 101, 192), 1));

        painter->drawRect(resizeHandle());
        painter->drawLine(QPointF(m_width / 2.0, 0), QPointF(m_width / 2.0, -24));
        painter->drawEllipse(rotateHandle());
    }
}

void ObjectItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (isSelected()) {
        if (resizeHandle().contains(event->pos())) {
            setCursor(Qt::SizeFDiagCursor);
        } else if (rotateHandle().contains(event->pos())) {
            setCursor(Qt::ClosedHandCursor);
        } else {
            setCursor(Qt::SizeAllCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
    BaseEditorItem::hoverMoveEvent(event);
}

void ObjectItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelected()) {
        if (resizeHandle().contains(event->pos())) {
            m_state = StateResize;
            return;
        } else if (rotateHandle().contains(event->pos())) {
            m_state = StateRotate;
            return;
        }
    }
    m_state = StateNone;
    BaseEditorItem::mousePressEvent(event);
}

void ObjectItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_state == StateResize) {
        int gridSize = 5;

        qreal rawWidth = qMax(20.0, event->pos().x());
        qreal rawLength = qMax(20.0, event->pos().y());

        qreal newWidth = qRound(rawWidth / gridSize) * gridSize;
        qreal newLength = qRound(rawLength / gridSize) * gridSize;

        if (qAbs(m_width - newWidth) < 1e-5 && qAbs(m_length - newLength) < 1e-5) {
            return;
        }

        prepareGeometryChange();

        QPointF topLeftSceneBefore = mapToScene(0, 0);

        m_width = newWidth;
        m_length = newLength;
        setTransformOriginPoint(m_width / 2.0, m_length / 2.0);

        QPointF topLeftSceneAfter = mapToScene(0, 0);

        setPos(pos() + (topLeftSceneBefore - topLeftSceneAfter));

        if (scene()) scene()->update();
        emit itemChanged();

    } else if (m_state == StateRotate) {
        QPointF centerInScene = mapToScene(m_width / 2.0, m_length / 2.0);
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

        if (qAbs(rotation() - angleDeg) > 0.1) {
            setRotation(angleDeg);
            emit itemChanged();
        }
    } else {
        BaseEditorItem::mouseMoveEvent(event);
    }
}

void ObjectItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_state = StateNone;
    setCursor(Qt::ArrowCursor);
    BaseEditorItem::mouseReleaseEvent(event);
}

void ObjectItem::setWidthInMeters(qreal widthMeters)
{
    if (qAbs(widthInMeters() - widthMeters) < 1e-5) return;
    prepareGeometryChange();

    QPointF topLeftSceneBefore = mapToScene(0, 0);

    m_width = widthMeters * 100.0;
    setTransformOriginPoint(m_width / 2.0, m_length / 2.0);

    QPointF topLeftSceneAfter = mapToScene(0, 0);
    setPos(pos() + (topLeftSceneBefore - topLeftSceneAfter));

    update();
    emit itemChanged();
}

void ObjectItem::setLengthInMeters(qreal lengthMeters)
{
    if (qAbs(lengthInMeters() - lengthMeters) < 1e-5) return;
    prepareGeometryChange();

    QPointF topLeftSceneBefore = mapToScene(0, 0);

    m_length = lengthMeters * 100.0;
    setTransformOriginPoint(m_width / 2.0, m_length / 2.0);

    QPointF topLeftSceneAfter = mapToScene(0, 0);
    setPos(pos() + (topLeftSceneBefore - topLeftSceneAfter));

    update();
    emit itemChanged();
}

void ObjectItem::setObjectType(int type)
{
    if (m_objectType == type) return;
    m_objectType = type;
    update();
    emit itemChanged();
}

qreal ObjectItem::widthInMeters() const { return m_width / 100.0; }
qreal ObjectItem::lengthInMeters() const { return m_length / 100.0; }
int ObjectItem::objectType() const { return m_objectType; }

qreal ObjectItem::area() const
{
    return widthInMeters() * lengthInMeters();
}

QVariant ObjectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange && scene()) {
        QPointF newPos = value.toPointF();
        int gridSize = 5;
        qreal x = qRound(newPos.x() / gridSize) * gridSize;
        qreal y = qRound(newPos.y() / gridSize) * gridSize;
        return QPointF(x, y);
    }
    return BaseEditorItem::itemChange(change, value);
}

QJsonObject ObjectItem::toJson() const
{
    QJsonObject json = BaseEditorItem::toJson();
    json["width"] = m_width;
    json["length"] = m_length;
    json["material_name"] = m_materialName;
    return json;
}

void ObjectItem::fromJson(const QJsonObject &json)
{
    BaseEditorItem::fromJson(json);
    m_width = json["width"].toDouble();
    m_length = json["length"].toDouble();
    m_materialName = json["material_name"].toString();

    setTransformOriginPoint(m_width / 2.0, m_length / 2.0);
    update();
}

void ObjectItem::setMaterialName(const QString &name)
{
    if (m_materialName == name) return;
    m_materialName = name;
    update();
    emit itemChanged();
}

QString ObjectItem::materialName() const { return m_materialName; }
