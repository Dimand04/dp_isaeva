#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include "editorscene.h"
#include "baseeditoritem.h"
#include <QLabel>
#include <QStandardItemModel>
#include "roofitem.h"

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
    void onNodePropertyChanged();
    void onDimensionPropertyChanged();
    void onTextPropertyChanged();
    void onItemAddedToScene(BaseEditorItem *item);
    void onItemChangedInScene();
    void onTreeItemClicked(const QModelIndex &index);
    void onFloorPropertyChanged();
    void onRoofPropertyChanged();

private:
    Ui::EditorWindow *ui;
    int m_projectId;
    EditorScene *m_scene;
    BaseEditorItem *m_trackedItem = nullptr;
    QLabel *m_coordLabel;
    QStandardItemModel *m_treeModel;
    QStandardItem* getOrCreateLayerNode(int levelId, const QString &layerName);
};

Q_DECLARE_METATYPE(BaseEditorItem*)

#endif
