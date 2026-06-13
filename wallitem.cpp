#include "wallitem.h"
#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QtMath>
#include <QGraphicsScene>
#include <QPainterPath>
#include <QPainterPathStroker>
#include "windowitem.h"
#include "dooritem.h"

WallItem::WallItem(const QPointF &startPoint, QGraphicsItem *parent)
    : BaseEditorItem(parent), m_thickness(20.0), m_isUpdating(false), m_state(WallStateNone)
{
    setAcceptHoverEvents(true);
    m_line.setP1(QPointF(0, 0));
    m_line.setP2(QPointF(0, 0));
    setPos(startPoint);
    updatePolygon();
}

void WallItem::setEndPoint(const QPointF &endPoint)
{
    prepareGeometryChange();
    m_line.setP2(mapFromScene(endPoint));
    updatePolygon();
    update();
    emit itemChanged();
}

QRectF WallItem::startHandle() const
{
    return QRectF(-5, -5, 10, 10);
}

QRectF WallItem::endHandle() const
{
    return QRectF(m_line.p2().x() - 5, m_line.p2().y() - 5, 10, 10);
}

QRectF WallItem::boundingRect() const
{
    return m_polygon.boundingRect().adjusted(-10, -10, 10, 10);
}

QPainterPath WallItem::shape() const
{
    QPainterPath path;
    path.addPolygon(m_polygon);
    QPainterPathStroker stroker;
    stroker.setWidth(10);
    return path.united(stroker.createStroke(path));
}

void WallItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(QColor(230, 230, 230));
    painter->setPen(QPen(QColor(50, 50, 50), 2, Qt::SolidLine, Qt::RoundCap, Qt::MiterJoin));
    painter->drawPolygon(m_polygon);

    if (isSelected()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(QColor(21, 101, 192), 2, Qt::DashLine));
        painter->drawPolygon(m_polygon);

        painter->setBrush(Qt::white);
        painter->setPen(QPen(QColor(21, 101, 192), 1));
        painter->drawRect(startHandle());
        painter->drawRect(endHandle());
    }
}

void WallItem::setLengthInMeters(qreal lengthMeters)
{
    prepareGeometryChange();
    qreal lengthPx = lengthMeters * 100.0;
    qreal angleRad = qDegreesToRadians(m_line.angle());
    qreal newX = m_line.p1().x() + lengthPx * qCos(angleRad);
    qreal newY = m_line.p1().y() - lengthPx * qSin(angleRad);
    m_line.setP2(QPointF(newX, newY));
    updatePolygon();
    update();
    emit itemChanged();
}

void WallItem::setThicknessInMm(qreal thicknessMm)
{
    prepareGeometryChange();
    m_thickness = thicknessMm * 0.1;
    updatePolygon();
    update();
    emit itemChanged();
}

void WallItem::setAngleInDegrees(qreal angleDeg)
{
    prepareGeometryChange();
    m_line.setAngle(angleDeg);
    updatePolygon();
    update();
    emit itemChanged();
}

qreal WallItem::lengthInMeters() const
{
    return m_line.length() / 100.0;
}

qreal WallItem::thicknessInMm() const
{
    return m_thickness / 0.1;
}

qreal WallItem::angleInDegrees() const
{
    qreal angle = m_line.angle();
    if (angle < 0) angle += 360;
    return angle;
}

void WallItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (isSelected()) {
        if (startHandle().contains(event->pos()) || endHandle().contains(event->pos())) {
            setCursor(Qt::SizeAllCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    } else {
        setCursor(Qt::ArrowCursor);
    }
    BaseEditorItem::hoverMoveEvent(event);
}

void WallItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelected()) {
        if (startHandle().contains(event->pos())) {
            m_state = WallStateMoveStart;
            return;
        } else if (endHandle().contains(event->pos())) {
            m_state = WallStateMoveEnd;
            return;
        }
    }
    m_state = WallStateNone;
    BaseEditorItem::mousePressEvent(event);
}

void WallItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    int gridSize = 5;

    if (m_state == WallStateMoveEnd) {
        QPointF snappedScenePos = event->scenePos();
        snappedScenePos.setX(qRound(snappedScenePos.x() / gridSize) * gridSize);
        snappedScenePos.setY(qRound(snappedScenePos.y() / gridSize) * gridSize);

        if (event->modifiers() & Qt::ShiftModifier) {
            QPointF startP = pos();
            if (qAbs(snappedScenePos.x() - startP.x()) > qAbs(snappedScenePos.y() - startP.y())) {
                snappedScenePos.setY(startP.y());
            } else {
                snappedScenePos.setX(startP.x());
            }
        }

        prepareGeometryChange();
        m_line.setP2(mapFromScene(snappedScenePos));

        updatePolygon();
        if (scene()) scene()->update();
        emit itemChanged();

    } else if (m_state == WallStateMoveStart) {
        QPointF oldEndScene = mapToScene(m_line.p2());

        QPointF snappedStartScene = event->scenePos();
        snappedStartScene.setX(qRound(snappedStartScene.x() / gridSize) * gridSize);
        snappedStartScene.setY(qRound(snappedStartScene.y() / gridSize) * gridSize);

        if (event->modifiers() & Qt::ShiftModifier) {
            if (qAbs(snappedStartScene.x() - oldEndScene.x()) > qAbs(snappedStartScene.y() - oldEndScene.y())) {
                snappedStartScene.setY(oldEndScene.y());
            } else {
                snappedStartScene.setX(oldEndScene.x());
            }
        }

        prepareGeometryChange();
        setPos(snappedStartScene);
        m_line.setP2(mapFromScene(oldEndScene));

        updatePolygon();
        if (scene()) scene()->update();
        emit itemChanged();

    } else {
        BaseEditorItem::mouseMoveEvent(event);
    }
}

void WallItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_state = WallStateNone;
    BaseEditorItem::mouseReleaseEvent(event);
}

QVariant WallItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged || change == ItemTransformHasChanged) {
        updatePolygon();
    }
    return BaseEditorItem::itemChange(change, value);
}

void WallItem::updatePolygon()
{
    if (m_isUpdating) return;
    m_isUpdating = true;

    QPointF p1 = m_line.p1();
    QPointF p2 = m_line.p2();

    QLineF axis = m_line;
    QLineF normal = axis.normalVector();
    if (normal.length() == 0) { m_isUpdating = false; return; }
    normal.setLength(1.0);
    QPointF dir(normal.dx(), normal.dy());

    qreal leftDist = m_thickness / 2.0;
    qreal rightDist = m_thickness / 2.0;
    if (m_alignment == 1) { leftDist = 0.0; rightDist = m_thickness; }
    else if (m_alignment == 2) { leftDist = m_thickness; rightDist = 0.0; }

    // Базовые (исходные) координаты прямоугольника до любых подрезок
    QPointF p1L_orig = p1 + dir * leftDist;
    QPointF p1R_orig = p1 - dir * rightDist;
    QPointF p2L_orig = p2 + dir * leftDist;
    QPointF p2R_orig = p2 - dir * rightDist;

    QPointF p1L = p1L_orig;
    QPointF p1R = p1R_orig;
    QPointF p2L = p2L_orig;
    QPointF p2R = p2R_orig;

    QPolygonF myBasePoly;
    myBasePoly << p1L_orig << p2L_orig << p2R_orig << p1R_orig;
    QPolygonF myBaseScene = mapToScene(myBasePoly);

    QSet<WallItem*> currentlyTouching;

    if (scene()) {
        QLineF myAxisScene(mapToScene(p1), mapToScene(p2));

        // Лямбда для поиска идеальной точки среза на углах
        auto getMiterPoint = [](QLineF myEdge, QLineF oEdge1, QLineF oEdge2, QPointF origPt) -> QPointF {
            QPointF i1, i2;
            bool b1 = myEdge.intersects(oEdge1, &i1) == QLineF::UnboundedIntersection;
            bool b2 = myEdge.intersects(oEdge2, &i2) == QLineF::UnboundedIntersection;
            if (!b1 && !b2) return origPt;
            if (b1 && !b2) return i1;
            if (!b1 && b2) return i2;
            return (QLineF(origPt, i1).length() < QLineF(origPt, i2).length()) ? i1 : i2;
        };

        for (QGraphicsItem* item : scene()->items()) {
            if (item == this) continue;
            WallItem* other = dynamic_cast<WallItem*>(item);
            if (!other) continue;

            QLineF oAxis = other->line();
            QLineF oNormal = oAxis.normalVector();
            if (oNormal.length() == 0) continue;
            oNormal.setLength(1.0);
            QPointF oDir(oNormal.dx(), oNormal.dy());

            qreal oThick = other->thicknessInMm() * 0.1;
            qreal oLeftDist = oThick / 2.0;
            qreal oRightDist = oThick / 2.0;
            if (other->alignment() == 1) { oLeftDist = 0.0; oRightDist = oThick; }
            else if (other->alignment() == 2) { oLeftDist = oThick; oRightDist = 0.0; }

            QLineF oLeft(oAxis.p1() + oDir * oLeftDist, oAxis.p2() + oDir * oLeftDist);
            QLineF oRight(oAxis.p1() - oDir * oRightDist, oAxis.p2() - oDir * oRightDist);

            QPolygonF oBasePoly;
            oBasePoly << oLeft.p1() << oLeft.p2() << oRight.p2() << oRight.p1();
            QPolygonF oBaseScene = other->mapToScene(oBasePoly);

            // 1. Если физически не касаются друг друга — игнорируем
            if (myBaseScene.intersected(oBaseScene).isEmpty()) continue;

            // 2. Ищем пересечение осей
            QLineF oAxisScene(other->mapToScene(oAxis.p1()), other->mapToScene(oAxis.p2()));
            QPointF axisInt;
            if (myAxisScene.intersects(oAxisScene, &axisInt) == QLineF::NoIntersection) continue;

            currentlyTouching.insert(other);

            QPointF sp1 = myAxisScene.p1();
            QPointF sp2 = myAxisScene.p2();
            QPointF osp1 = oAxisScene.p1();
            QPointF osp2 = oAxisScene.p2();

            qreal distToSp1 = QLineF(sp1, axisInt).length();
            qreal distToSp2 = QLineF(sp2, axisInt).length();
            qreal distToOsp1 = QLineF(osp1, axisInt).length();
            qreal distToOsp2 = QLineF(osp2, axisInt).length();

            // Допуски для определения конца стены
            qreal threshThis = m_thickness * 3.0;
            qreal threshOther = oThick * 3.0;

            bool thisEnd1 = distToSp1 <= threshThis && distToSp1 <= distToSp2;
            bool thisEnd2 = distToSp2 <= threshThis && distToSp2 < distToSp1;
            bool otherIsEnd = distToOsp1 <= threshOther || distToOsp2 <= threshOther;

            QLineF mLeftScene(mapToScene(p1L_orig), mapToScene(p2L_orig));
            QLineF mRightScene(mapToScene(p1R_orig), mapToScene(p2R_orig));
            QLineF oLeftScene(other->mapToScene(oLeft.p1()), other->mapToScene(oLeft.p2()));
            QLineF oRightScene(other->mapToScene(oRight.p1()), other->mapToScene(oRight.p2()));

            // Обработка стыка на точке P1
            if (thisEnd1) {
                if (otherIsEnd) {
                    // Это УГОЛ
                    p1L = mapFromScene(getMiterPoint(mLeftScene, oLeftScene, oRightScene, mapToScene(p1L_orig)));
                    p1R = mapFromScene(getMiterPoint(mRightScene, oLeftScene, oRightScene, mapToScene(p1R_orig)));
                } else {
                    // Это Т-СТЫК (мы упираемся в чужую стену плоско)
                    QPointF iL, iR;
                    bool bL = myAxisScene.intersects(oLeftScene, &iL) == QLineF::UnboundedIntersection;
                    bool bR = myAxisScene.intersects(oRightScene, &iR) == QLineF::UnboundedIntersection;

                    // Находим ближайшую к нам грань другой стены
                    QLineF targetEdge = (bL && bR && QLineF(sp1, iL).length() < QLineF(sp1, iR).length()) ? oLeftScene : oRightScene;
                    if (!bL && bR) targetEdge = oRightScene;
                    if (bL && !bR) targetEdge = oLeftScene;

                    // Обрезаем ОБЕ наши грани строго по этой одной линии
                    if (!targetEdge.isNull()) {
                        QPointF tempL, tempR;
                        if (mLeftScene.intersects(targetEdge, &tempL) == QLineF::UnboundedIntersection) p1L = mapFromScene(tempL);
                        if (mRightScene.intersects(targetEdge, &tempR) == QLineF::UnboundedIntersection) p1R = mapFromScene(tempR);
                    }
                }
            }

            // Обработка стыка на точке P2
            if (thisEnd2) {
                if (otherIsEnd) {
                    // Это УГОЛ
                    p2L = mapFromScene(getMiterPoint(mLeftScene, oLeftScene, oRightScene, mapToScene(p2L_orig)));
                    p2R = mapFromScene(getMiterPoint(mRightScene, oLeftScene, oRightScene, mapToScene(p2R_orig)));
                } else {
                    // Это Т-СТЫК
                    QPointF iL, iR;
                    bool bL = myAxisScene.intersects(oLeftScene, &iL) == QLineF::UnboundedIntersection;
                    bool bR = myAxisScene.intersects(oRightScene, &iR) == QLineF::UnboundedIntersection;

                    QLineF targetEdge = (bL && bR && QLineF(sp2, iL).length() < QLineF(sp2, iR).length()) ? oLeftScene : oRightScene;
                    if (!bL && bR) targetEdge = oRightScene;
                    if (bL && !bR) targetEdge = oLeftScene;

                    if (!targetEdge.isNull()) {
                        QPointF tempL, tempR;
                        if (mLeftScene.intersects(targetEdge, &tempL) == QLineF::UnboundedIntersection) p2L = mapFromScene(tempL);
                        if (mRightScene.intersects(targetEdge, &tempR) == QLineF::UnboundedIntersection) p2R = mapFromScene(tempR);
                    }
                }
            }
        }
    }

    // Защита от перекручивания (Anti-Bowtie)
    auto checkCross = [](QPointF oL, QPointF oR, QPointF nL, QPointF nR) {
        QLineF origLine(oL, oR);
        QLineF newLine(nL, nR);
        qreal angle = qAbs(origLine.angleTo(newLine));
        return (angle > 90.0 && angle < 270.0);
    };
    if (checkCross(p1L_orig, p1R_orig, p1L, p1R)) { p1L = p1L_orig; p1R = p1R_orig; }
    if (checkCross(p2L_orig, p2R_orig, p2L, p2R)) { p2L = p2L_orig; p2R = p2R_orig; }

    // Сборка финального полигона
    if (m_polygon.isEmpty() || m_polygon[0] != p1L || m_polygon[1] != p2L || m_polygon[2] != p2R || m_polygon[3] != p1R) {
        prepareGeometryChange();
        m_polygon.clear();
        m_polygon << p1L << p2L << p2R << p1R;
    }

    // Автоматическое обновление соседей
    QSet<WallItem*> detachedWalls = m_connectedWalls;
    detachedWalls.subtract(currentlyTouching);
    m_connectedWalls = currentlyTouching;

    for (WallItem* detached : detachedWalls) {
        if (detached->scene() == scene()) {
            detached->updatePolygon();
            detached->update();
        }
    }
    for (WallItem* attached : currentlyTouching) {
        if (attached->scene() == scene()) {
            attached->updatePolygon();
            attached->update();
        }
    }

    m_isUpdating = false;
}

qreal WallItem::calculateExactArea() const
{
    if (m_polygon.isEmpty()) return 0.0;
    qreal area = 0.0;
    int j = m_polygon.count() - 1;
    for (int i = 0; i < m_polygon.count(); i++) {
        area += (m_polygon[j].x() + m_polygon[i].x()) * (m_polygon[j].y() - m_polygon[i].y());
        j = i;
    }
    return qAbs(area / 2.0) / 10000.0;
}

qreal WallItem::netArea() const
{
    qreal grossArea = calculateExactArea();
    qreal deductions = 0.0;

    if (scene()) {
        for (QGraphicsItem *item : scene()->items()) {
            if (WindowItem *w = dynamic_cast<WindowItem*>(item)) {
                if (w->hostWall() == this) {
                    deductions += w->widthInMeters() * (thicknessInMm() / 1000.0);
                }
            } else if (DoorItem *d = dynamic_cast<DoorItem*>(item)) {
                if (d->hostWall() == this) {
                    deductions += d->widthInMeters() * (thicknessInMm() / 1000.0);
                }
            }
        }
    }
    return qMax(0.0, grossArea - deductions);
}

int WallItem::alignment() const
{
    return m_alignment;
}

void WallItem::setAlignment(int alignment)
{
    if (m_alignment == alignment) return;
    m_alignment = alignment;
    updatePolygon();
    update();
    emit itemChanged();
}

qreal WallItem::actualLength() const
{
    qreal thickM = thicknessInMm() / 1000.0;
    if (thickM <= 0) return lengthInMeters();

    qreal grossArea = calculateExactArea();
    return grossArea / thickM;
}
