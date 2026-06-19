#include "editorscene.h"
#include "foundationblockitem.h"
#include <QPainter>
#include <QPen>
#include "wallitem.h"
#include <QtMath>
#include "windowitem.h"
#include "dooritem.h"
#include "nodeitem.h"
#include "flooritem.h"
#include <QMenu>
#include <QAction>
#include <QSet>
#include "dimensionitem.h"
#include "textitem.h"
#include "objectitem.h"

EditorScene::EditorScene(QObject *parent)
    : QGraphicsScene(parent), m_currentMode(ModeCursor), m_isDrawing(false),
    m_currentWall(nullptr), m_currentFloor(nullptr), m_currentRoof(nullptr), m_gridSize(20)
{
    setSceneRect(0, 0, 2000, 2000);
}

void EditorScene::setToolMode(ToolMode mode)
{
    if (m_currentMode == ModeFloor && m_currentFloor && m_currentFloor->isDrawing()) {
        m_currentFloor->finishDrawing();
    }
    if (m_currentMode == ModeDimension && m_currentDimension && m_currentDimension->isDrawing()) {
        removeItem(m_currentDimension);
        delete m_currentDimension;
    }

    if (m_currentMode == ModeRoof && m_currentRoof && m_currentRoof->isDrawing()) {
        m_currentRoof->finishDrawing();
    }

    m_currentMode = mode;
    m_isDrawing = false;
    m_currentFloor = nullptr;
    m_currentWall = nullptr;
    m_currentDimension = nullptr;
    m_currentRoof = nullptr;
    clearSelection();

    emit toolModeChanged(mode);
}

void EditorScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->save();

    painter->setBrush(QColor(210, 210, 210));
    painter->setPen(Qt::NoPen);
    painter->drawRect(rect);

    QRectF workspace = sceneRect();
    painter->setBrush(Qt::white);
    painter->drawRect(workspace);

    qreal left = qMax(0.0, rect.left());
    qreal right = qMin(workspace.right(), rect.right());
    qreal top = qMax(0.0, rect.top());
    qreal bottom = qMin(workspace.bottom(), rect.bottom());

    if (left < right && top < bottom) {
        int gridSize = 5;
        int majorGridSize = 100;

        int startX = int(left) - (int(left) % gridSize);
        int startY = int(top) - (int(top) % gridSize);

        QList<QLineF> minorLines;
        QList<QLineF> majorLines;

        for (int x = startX; x <= right; x += gridSize) {
            if (x % majorGridSize == 0) majorLines.append(QLineF(x, top, x, bottom));
            else minorLines.append(QLineF(x, top, x, bottom));
        }
        for (int y = startY; y <= bottom; y += gridSize) {
            if (y % majorGridSize == 0) majorLines.append(QLineF(left, y, right, y));
            else minorLines.append(QLineF(left, y, right, y));
        }

        painter->setPen(QPen(QColor(240, 240, 240), 1));
        painter->drawLines(minorLines);

        painter->setPen(QPen(QColor(210, 210, 210), 1));
        painter->drawLines(majorLines);
    }

    painter->setBrush(Qt::NoBrush);
    painter->setPen(QPen(Qt::black, 2));
    painter->drawRect(workspace);

    painter->restore();
}

void EditorScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QPointF snappedPos = snapToGrid(event->scenePos());

        auto generateUniqueName = [this](const QString &baseName) -> QString {
            int index = 1;
            while (true) {
                QString testName = baseName + "_" + QString::number(index);
                bool nameExists = false;
                for (QGraphicsItem *item : items()) {
                    if (BaseEditorItem *baseItem = dynamic_cast<BaseEditorItem*>(item)) {
                        if (baseItem->name() == testName) {
                            nameExists = true;
                            break;
                        }
                    }
                }
                if (!nameExists) {
                    return testName;
                }
                index++;
            }
        };

        if (m_currentMode == ModeFoundation) {
            FoundationBlockItem *block = new FoundationBlockItem(200, 160);
            block->setName(generateUniqueName("Фундамент"));
            block->setPos(snappedPos);
            block->setLevelId(m_activeLevelId);
            block->setLayerName(m_activeLayerName);
            addItem(block);
            emit itemAdded(block);
            return;
        }
        else if (m_currentMode == ModeWall) {
            m_isDrawing = true;
            m_currentWall = new WallItem(snappedPos);
            m_currentWall->setName(generateUniqueName("Стена"));
            m_currentWall->setPos(snappedPos);
            m_currentWall->setLevelId(m_activeLevelId);
            m_currentWall->setLayerName(m_activeLayerName);
            addItem(m_currentWall);
            return;
        }
        else if (m_currentMode == ModeWindow) {
            WallItem *targetWall = nullptr;
            for (QGraphicsItem *item : items(event->scenePos())) {
                if (WallItem *wall = dynamic_cast<WallItem*>(item)) {
                    if (wall->levelId() == m_activeLevelId) {
                        targetWall = wall;
                        break;
                    }
                }
            }
            if (targetWall) {
                WindowItem *window = new WindowItem();
                window->setName(generateUniqueName("Окно"));
                window->setHostWall(targetWall);
                window->setWidthInMeters(1.5);
                window->setLevelId(m_activeLevelId);
                window->setLayerName(m_activeLayerName);
                addItem(window);
                window->setPos(event->scenePos());
                emit itemAdded(window);
            }
            return;
        }
        else if (m_currentMode == ModeDoor) {
            WallItem *targetWall = nullptr;
            for (QGraphicsItem *item : items(event->scenePos())) {
                if (WallItem *wall = dynamic_cast<WallItem*>(item)) {
                    if (wall->levelId() == m_activeLevelId) {
                        targetWall = wall;
                        break;
                    }
                }
            }

            if (targetWall) {
                DoorItem *door = new DoorItem();
                door->setName(generateUniqueName("Дверь"));
                door->setHostWall(targetWall);
                door->setWidthInMeters(0.9);
                door->setLevelId(m_activeLevelId);
                door->setLayerName(m_activeLayerName);
                addItem(door);
                door->setPos(event->scenePos());
                emit itemAdded(door);
            }
            return;
        }
        else if (m_currentMode == ModeNode) {
            NodeItem *node = new NodeItem(snappedPos);
            node->setName(generateUniqueName("Узел"));
            node->setLevelId(m_activeLevelId);
            node->setLayerName(m_activeLayerName);
            addItem(node);
            emit itemAdded(node);
            clearSelection();
            node->setSelected(true);
            return;
        }
        else if (m_currentMode == ModeFloor) {
            if (!m_currentFloor || !m_currentFloor->isDrawing()) {
                m_currentFloor = new FloorItem();
                m_currentFloor->setName(generateUniqueName("Пол"));
                m_currentFloor->setLevelId(m_activeLevelId);
                m_currentFloor->setLayerName(m_activeLayerName);
                addItem(m_currentFloor);
                emit itemAdded(m_currentFloor);
                m_currentFloor->addPoint(snappedPos);
            } else {
                m_currentFloor->addPoint(snappedPos);
            }
            return;
        }
        else if (m_currentMode == ModeRoof) {
            if (!m_currentRoof || !m_currentRoof->isDrawing()) {
                m_currentRoof = new RoofItem();
                m_currentRoof->setName(generateUniqueName("Крыша"));
                m_currentRoof->setLevelId(m_activeLevelId);
                m_currentRoof->setLayerName(m_activeLayerName);
                addItem(m_currentRoof);
                emit itemAdded(m_currentRoof);
                m_currentRoof->addPoint(snappedPos);
            } else {
                m_currentRoof->addPoint(snappedPos);
            }
            return;
        }
        else if (m_currentMode == ModeDimension) {
            if (!m_currentDimension) {
                m_isDrawing = true;
                m_currentDimension = new DimensionItem();
                m_currentDimension->setName(generateUniqueName("Размер"));
                m_currentDimension->setLevelId(m_activeLevelId);
                m_currentDimension->setLayerName(m_activeLayerName);
                addItem(m_currentDimension);
                m_currentDimension->setStartPoint(snappedPos);
            } else {
                m_currentDimension->setEndPoint(snappedPos);
                m_currentDimension->finishDrawing();
                emit itemAdded(m_currentDimension);
                m_currentDimension = nullptr;
                m_isDrawing = false;
            }
            return;
        }
        else if (m_currentMode == ModeText) {
            TextItem *textItem = new TextItem();
            textItem->setName(generateUniqueName("Метка"));
            textItem->setLevelId(m_activeLevelId);
            textItem->setLayerName(m_activeLayerName);
            addItem(textItem);
            textItem->setPos(snappedPos);
            emit itemAdded(textItem);
            clearSelection();
            textItem->setSelected(true);
            setToolMode(ModeCursor);
            return;
        }
        else if (m_currentMode == ModeObject) {
            ObjectItem *obj = new ObjectItem();
            obj->setName(generateUniqueName("Мебель"));
            obj->setPos(snappedPos);
            obj->setLevelId(m_activeLevelId);
            obj->setLayerName(m_activeLayerName);
            addItem(obj);
            emit itemAdded(obj);

            clearSelection();
            obj->setSelected(true);

            setToolMode(ModeCursor);
            return;
        }
    }
    else if (event->button() == Qt::RightButton) {
        if (m_currentMode == ModeFloor && m_currentFloor && m_currentFloor->isDrawing()) {
            m_currentFloor->removeLastPoint();
            if (m_currentFloor->polygonSize() < 2) {
                removeItem(m_currentFloor);
                delete m_currentFloor;
                m_currentFloor = nullptr;
            }
            event->accept();
            return;
        }
        if (m_currentMode == ModeRoof && m_currentRoof && m_currentRoof->isDrawing()) {
            m_currentRoof->removeLastPoint();
            if (m_currentRoof->polygonSize() < 2) {
                removeItem(m_currentRoof);
                delete m_currentRoof;
                m_currentRoof = nullptr;
            }
            event->accept();
            return;
        }
        if (m_currentMode == ModeDimension && m_isDrawing && m_currentDimension) {
            removeItem(m_currentDimension);
            delete m_currentDimension;
            m_currentDimension = nullptr;
            m_isDrawing = false;
            event->accept();
            return;
        }
    }

    QGraphicsScene::mousePressEvent(event);
}

void EditorScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF snappedPos = snapToGrid(event->scenePos());

    emit cursorMoved(snappedPos);

    if (m_currentMode == ModeWall && m_isDrawing && m_currentWall) {
        m_currentWall->setEndPoint(snappedPos);
    }
    else if (m_currentMode == ModeFloor && m_currentFloor && m_currentFloor->isDrawing()) {
        m_currentFloor->setDynamicPoint(snappedPos);
    }
    else if (m_currentMode == ModeDimension && m_isDrawing && m_currentDimension) {
        m_currentDimension->setEndPoint(snappedPos);
    }
    else if (m_currentMode == ModeRoof && m_currentRoof && m_currentRoof->isDrawing()) {
        m_currentRoof->setDynamicPoint(snappedPos);
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void EditorScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isDrawing && m_currentMode == ModeWall) {
        m_isDrawing = false;

        if (m_currentWall && m_currentWall->line().length() < 10) {
            removeItem(m_currentWall);
            delete m_currentWall;
        } else if (m_currentWall) {
            clearSelection();
            m_currentWall->setSelected(true);

            emit itemAdded(m_currentWall);
        }

        m_currentWall = nullptr;
    }

    QGraphicsScene::mouseReleaseEvent(event);
}

QPointF EditorScene::snapToGrid(const QPointF &pos)
{
    qreal x = qFloor(pos.x() / m_gridSize) * m_gridSize;
    qreal y = qFloor(pos.y() / m_gridSize) * m_gridSize;
    return QPointF(x, y);
}

int EditorScene::toolMode() const
{
    return m_currentMode;
}

void EditorScene::deleteSelectedItems()
{
    QList<QGraphicsItem*> selected = selectedItems();
    if (selected.isEmpty()) return;

    QSet<QGraphicsItem*> itemsToDelete;

    for (QGraphicsItem* item : selected) {
        itemsToDelete.insert(item);

        if (WallItem* wall = dynamic_cast<WallItem*>(item)) {
            for (QGraphicsItem* sceneItem : items()) {
                if (WindowItem* w = dynamic_cast<WindowItem*>(sceneItem)) {
                    if (w->hostWall() == wall) itemsToDelete.insert(w);
                } else if (DoorItem* d = dynamic_cast<DoorItem*>(sceneItem)) {
                    if (d->hostWall() == wall) itemsToDelete.insert(d);
                }
            }
        }
    }

    for (QGraphicsItem* item : itemsToDelete) {
        if (item == m_currentFloor) m_currentFloor = nullptr;
        if (item == m_currentRoof) m_currentRoof = nullptr;
        removeItem(item);
        delete item;
    }

    if (!itemsToDelete.isEmpty()) {
        emit itemDeleted();
    }
}

void EditorScene::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        deleteSelectedItems();
        event->accept();
    } else {
        QGraphicsScene::keyPressEvent(event);
    }
}

void EditorScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (m_currentMode == ModeFloor || m_currentMode == ModeRoof) {
        event->accept();
        return;
    }

    QGraphicsItem *item = itemAt(event->scenePos(), QTransform());

    if (item) {
        QMenu menu;
        QAction *deleteAction = menu.addAction("Удалить");
        QAction *continueAction = nullptr;
        QAction *deletePointAction = nullptr;

        FloorItem *floor = dynamic_cast<FloorItem*>(item);
        RoofItem *roof = dynamic_cast<RoofItem*>(item);

        int vIndex = -1;

        if (floor && !floor->isDrawing()) {
            continueAction = menu.addAction("Продолжить");

            vIndex = floor->vertexAt(floor->mapFromScene(event->scenePos()));
            if (vIndex != -1) {
                deletePointAction = menu.addAction("Удалить точку");
                if (floor->polygonSize() <= 3) {
                    deletePointAction->setEnabled(false);
                }
            }
        }
        else if (roof && !roof->isDrawing()) {
            continueAction = menu.addAction("Продолжить");

            vIndex = roof->vertexAt(roof->mapFromScene(event->scenePos()));
            if (vIndex != -1) {
                deletePointAction = menu.addAction("Удалить точку");
                if (roof->polygonSize() <= 3) {
                    deletePointAction->setEnabled(false);
                }
            }
        }

        QAction *selectedAction = menu.exec(event->screenPos());

        if (selectedAction) {
            if (selectedAction == deleteAction) {
                if (!item->isSelected()) {
                    clearSelection();
                    item->setSelected(true);
                }
                deleteSelectedItems();
            }
            else if (continueAction && selectedAction == continueAction) {
                if (floor) {
                    setToolMode(ModeFloor);
                    m_currentFloor = floor;
                    m_currentFloor->resumeDrawing(snapToGrid(event->scenePos()));
                }
                else if (roof) {
                    setToolMode(ModeRoof);
                    m_currentRoof = roof;
                    m_currentRoof->resumeDrawing(snapToGrid(event->scenePos()));
                }
            }
            else if (deletePointAction && selectedAction == deletePointAction) {
                if (floor) floor->removePoint(vIndex);
                if (roof) roof->removePoint(vIndex);
            }
        }
        event->accept();
    } else {
        QGraphicsScene::contextMenuEvent(event);
    }
}

void EditorScene::setWorkspaceSize(qreal widthMeters, qreal heightMeters)
{
    setSceneRect(0, 0, widthMeters * 100.0, heightMeters * 100.0);
    update();
}

QSizeF EditorScene::workspaceSize() const
{
    return QSizeF(sceneRect().width() / 100.0, sceneRect().height() / 100.0);
}

void EditorScene::setActiveLevel(int levelId)
{
    m_activeLevelId = levelId;
    updateItemsVisibility();
}

void EditorScene::setActiveLayer(const QString &layerName)
{
    m_activeLayerName = layerName;
}

void EditorScene::updateItemsVisibility()
{
    for (QGraphicsItem *gItem : items()) {
        if (BaseEditorItem *item = dynamic_cast<BaseEditorItem*>(gItem)) {

            if (m_hiddenLayers.contains(item->layerName())) {
                item->setVisible(false);
                continue;
            }

            if (item->levelId() == m_activeLevelId) {
                item->setVisible(true);
                item->setOpacity(1.0);
                item->setFlag(QGraphicsItem::ItemIsSelectable, true);
                item->setAcceptHoverEvents(true);
            }
            else if (item->levelId() < m_activeLevelId) {
                item->setVisible(true);
                item->setOpacity(0.3);
                item->setFlag(QGraphicsItem::ItemIsSelectable, false);
                item->setAcceptHoverEvents(false);
            }
            else {
                item->setVisible(false);
            }
        }
    }
}

void EditorScene::setLayerVisible(const QString &layerName, bool visible)
{
    if (visible) {
        m_hiddenLayers.remove(layerName);
    } else {
        m_hiddenLayers.insert(layerName);
    }
    updateItemsVisibility();
}
