#ifndef EDITORVIEW_H
#define EDITORVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>

class EditorView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit EditorView(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool m_isPanning;
    QPoint m_lastMousePos;
};

#endif
