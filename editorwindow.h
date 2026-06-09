#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include "editorscene.h"

namespace Ui {
class EditorWindow;
}

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorWindow(int projectId, QWidget *parent = nullptr);
    ~EditorWindow();

private slots:
    void onToolButtonClicked();

private:
    Ui::EditorWindow *ui;
    int m_projectId;
    EditorScene *m_scene;
};

#endif
