#ifndef EDITORSCENE_H
#define EDITORSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QKeyEvent>

enum ToolMode {
    ModeCursor,
    ModeFoundation,
    ModeWall,
    ModeNode,
    ModeWindow,
    ModeDoor,
    ModeFloor
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

signals:
    void cursorMoved(const QPointF &scenePos);

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
};

#endif
