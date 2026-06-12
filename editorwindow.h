#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include "editorscene.h"
#include "baseeditoritem.h"

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
    void onSelectionChanged();
    void onFoundationPropertyChanged();
    void onWallPropertyChanged();
    void onWindowPropertyChanged();
    void onDoorPropertyChanged();

private:
    Ui::EditorWindow *ui;
    int m_projectId;
    EditorScene *m_scene;
    BaseEditorItem *m_trackedItem = nullptr;
};

#endif
