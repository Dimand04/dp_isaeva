#include "editorview.h"
#include "editorscene.h"
#include <QPainter>
#include <QtMath>
#include <QScrollBar>

EditorView::EditorView(QWidget *parent)
    : QGraphicsView(parent), m_isPanning(false)
{
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    setMouseTracking(true);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
}

void EditorView::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        const double scaleFactor = 1.15;
        if (event->angleDelta().y() > 0) {
            scale(scaleFactor, scaleFactor);
        } else {
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
        }
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void EditorView::mousePressEvent(QMouseEvent *event)
{
    EditorScene *eScene = dynamic_cast<EditorScene*>(scene());

    if (event->button() == Qt::MiddleButton) {
        m_isPanning = true;
        m_lastPanPos = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    if (event->button() == Qt::LeftButton && eScene && eScene->toolMode() == 0) {
        QGraphicsItem *item = itemAt(event->pos());
        if (!item) {
            eScene->clearSelection();
            m_isPanning = true;
            m_lastPanPos = event->pos();
            setCursor(Qt::ClosedHandCursor);
            event->accept();
            return;
        }
    }

    QGraphicsView::mousePressEvent(event);
}

void EditorView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        QPoint delta = event->pos() - m_lastPanPos;
        horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
        verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
        m_lastPanPos = event->pos();
        event->accept();
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void EditorView::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isPanning) {
        m_isPanning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void EditorView::drawForeground(QPainter *painter, const QRectF &rect)
{
    Q_UNUSED(rect);
    painter->save();

    painter->setTransform(QTransform());

    int rSize = 25;
    int w = viewport()->width();
    int h = viewport()->height();

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(245, 245, 245, 230));
    painter->drawRect(0, 0, w, rSize);
    painter->drawRect(0, 0, rSize, h);

    painter->setPen(QPen(QColor(180, 180, 180), 1));
    painter->drawLine(0, rSize, w, rSize);
    painter->drawLine(rSize, 0, rSize, h);

    qreal scaleVal = transform().m11();
    QPointF topLeftScene = mapToScene(0, 0);

    qreal pxPerMeter = 100.0 * scaleVal;
    qreal majorStepMeters = 1.0;

    if (pxPerMeter < 15.0) majorStepMeters = 10.0;
    else if (pxPerMeter < 40.0) majorStepMeters = 5.0;
    else if (pxPerMeter < 80.0) majorStepMeters = 2.0;
    else if (pxPerMeter > 400.0) majorStepMeters = 0.1;
    else if (pxPerMeter > 200.0) majorStepMeters = 0.5;

    qreal majorStepScene = majorStepMeters * 100.0;
    qreal minorStepScene = majorStepScene / 10.0;

    QFont textFont = painter->font();
    textFont.setPointSize(8);
    painter->setFont(textFont);

    int startX_idx = qFloor(topLeftScene.x() / minorStepScene);
    for (int i = startX_idx; ; ++i) {
        qreal x = i * minorStepScene;
        int vx = mapFromScene(x, 0).x();
        if (vx > w) break;
        if (vx < rSize) continue;

        bool isMajor = (i % 10 == 0);

        if (isMajor) {
            painter->setPen(QPen(Qt::black, 1));
            painter->drawLine(vx, rSize - 10, vx, rSize);

            QString text;
            if (majorStepMeters >= 1.0) text = QString::number(x / 100.0, 'f', 0) + " м";
            else text = QString::number(qRound(x), 'f', 0) + " см";

            painter->drawText(vx + 3, rSize - 12, text);
        } else {
            painter->setPen(QPen(QColor(100, 100, 100), 1));
            painter->drawLine(vx, rSize - 4, vx, rSize);
        }
    }

    int startY_idx = qFloor(topLeftScene.y() / minorStepScene);
    for (int i = startY_idx; ; ++i) {
        qreal y = i * minorStepScene;
        int vy = mapFromScene(0, y).y();
        if (vy > h) break;
        if (vy < rSize) continue;

        bool isMajor = (i % 10 == 0);

        if (isMajor) {
            painter->setPen(QPen(Qt::black, 1));
            painter->drawLine(rSize - 10, vy, rSize, vy);

            QString text;
            if (majorStepMeters >= 1.0) text = QString::number(y / 100.0, 'f', 0) + " м";
            else text = QString::number(qRound(y), 'f', 0) + " см";

            painter->save();
            painter->translate(rSize - 12, vy - 3);
            painter->rotate(-90);
            painter->drawText(0, 0, text);
            painter->restore();
        } else {
            painter->setPen(QPen(QColor(100, 100, 100), 1));
            painter->drawLine(rSize - 4, vy, rSize, vy);
        }
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(235, 235, 235));
    painter->drawRect(0, 0, rSize, rSize);

    painter->restore();
}
