#ifndef EDITORSCENE_H
#define EDITORSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>
#include "baseeditoritem.h"
#include <QSet>
#include "roofitem.h"

enum ToolMode {
    ModeCursor,
    ModeFoundation,
    ModeWall,
    ModeNode,
    ModeWindow,
    ModeDoor,
    ModeFloor,
    ModeDimension,
    ModeText,
    ModeRoof,
    ModeObject
};

class EditorScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit EditorScene(QObject *parent = nullptr);
    void setToolMode(ToolMode mode);
    int toolMode() const;
    void deleteSelectedItems();
    void setWorkspaceSize(qreal widthMeters, qreal heightMeters);
    QSizeF workspaceSize() const;
    void setActiveLevel(int levelId);
    void setActiveLayer(const QString &layerName);
    void updateItemsVisibility();
    void setLayerVisible(const QString &layerName, bool visible);

signals:
    void cursorMoved(const QPointF &scenePos);
    void toolModeChanged(ToolMode mode);
    void itemAdded(BaseEditorItem *item);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    ToolMode m_currentMode;
    bool m_isDrawing;
    class WallItem *m_currentWall;
    class FloorItem *m_currentFloor;
    int m_gridSize;
    QPointF snapToGrid(const QPointF &pos);
    class DimensionItem *m_currentDimension;
    int m_activeLevelId = 1;
    QString m_activeLayerName = "Основной";
    QSet<QString> m_hiddenLayers;
    RoofItem *m_currentRoof = nullptr;
};

#endif
