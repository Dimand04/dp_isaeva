#include "editorscene.h"
#include "foundationblockitem.h"
#include <QPainter>
#include <QPen>
#include "wallitem.h"
#include <QtMath>
#include "windowitem.h"

EditorScene::EditorScene(QObject *parent)
    : QGraphicsScene(parent), m_currentMode(ModeCursor), m_isDrawing(false), m_currentWall(nullptr), m_gridSize(20)
{
    setSceneRect(-5000, -5000, 10000, 10000);
}

void EditorScene::setToolMode(ToolMode mode)
{
    m_currentMode = mode;
}

void EditorScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    qreal left = qFloor(rect.left() / m_gridSize) * m_gridSize;
    qreal top = qFloor(rect.top() / m_gridSize) * m_gridSize;

    QVector<QLineF> lines;
    for (qreal x = left; x < rect.right(); x += m_gridSize) {
        lines.append(QLineF(x, rect.top(), x, rect.bottom()));
    }
    for (qreal y = top; y < rect.bottom(); y += m_gridSize) {
        lines.append(QLineF(rect.left(), y, rect.right(), y));
    }

    painter->setPen(QPen(QColor(230, 230, 230), 1, Qt::SolidLine));
    painter->drawLines(lines);

    painter->setPen(QPen(QColor(180, 180, 180), 2, Qt::SolidLine));
    painter->drawLine(0, rect.top(), 0, rect.bottom());
    painter->drawLine(rect.left(), 0, rect.right(), 0);
}

void EditorScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF snappedPos = snapToGrid(event->scenePos());

        if (m_currentMode == ModeFoundation) {
            FoundationBlockItem *block = new FoundationBlockItem(200, 160);
            block->setPos(snappedPos);
            addItem(block);
            return;
        }
        else if (m_currentMode == ModeWall) {
            m_isDrawing = true;
            m_currentWall = new WallItem(snappedPos);
            m_currentWall->setPos(snappedPos);
            addItem(m_currentWall);
            return;
        } else if (m_currentMode == ModeWindow) {
            QGraphicsItem *clickedItem = itemAt(event->scenePos(), QTransform());

            if (WallItem *wall = dynamic_cast<WallItem*>(clickedItem)) {

                WindowItem *window = new WindowItem(150, wall);
                addItem(window);

                window->snapToPos(event->scenePos());

                window->setRotation(-wall->line().angle());
            }
            return;
        }
    }
    QGraphicsScene::mousePressEvent(event);
}

void EditorScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDrawing && m_currentWall && m_currentMode == ModeWall) {
        QPointF snappedPos = snapToGrid(event->scenePos());

        if (event->modifiers() & Qt::ShiftModifier) {
            QPointF startP = m_currentWall->pos();
            if (qAbs(snappedPos.x() - startP.x()) > qAbs(snappedPos.y() - startP.y())) {
                snappedPos.setY(startP.y());
            } else {
                snappedPos.setX(startP.x());
            }
        }

        m_currentWall->setEndPoint(snappedPos);
        return;
    }
    QGraphicsScene::mouseMoveEvent(event);
}

void EditorScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isDrawing) {
        m_isDrawing = false;

        if (m_currentWall && m_currentWall->line().length() < 10) {
            removeItem(m_currentWall);
            delete m_currentWall;
        }
        m_currentWall = nullptr;
        return;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

QPointF EditorScene::snapToGrid(const QPointF &pos)
{
    qreal x = qFloor(pos.x() / m_gridSize) * m_gridSize;
    qreal y = qFloor(pos.y() / m_gridSize) * m_gridSize;
    return QPointF(x, y);
}
