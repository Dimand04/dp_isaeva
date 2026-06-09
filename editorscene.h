#ifndef EDITORSCENE_H
#define EDITORSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

enum ToolMode {
    ModeCursor,
    ModeFoundation,
    ModeWall,
    ModeNode
};

class EditorScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit EditorScene(QObject *parent = nullptr);
    void setToolMode(ToolMode mode);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    ToolMode m_currentMode;
    bool m_isDrawing;
    class WallItem *m_currentWall;
    int m_gridSize;
    QPointF snapToGrid(const QPointF &pos);
};

#endif
