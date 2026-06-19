#ifndef EDITORWINDOW_H
#define EDITORWINDOW_H

#include <QMainWindow>
#include "editorscene.h"
#include "baseeditoritem.h"
#include <QLabel>
#include <QStandardItemModel>
#include <QCloseEvent>
#include <QList>
#include <QByteArray>
#include <QTimer>
#include <QShortcut>

namespace Ui {
class EditorWindow;
}

class EditorWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit EditorWindow(int projectId, QWidget *parent = nullptr);
    ~EditorWindow();

    void saveProject();
    void loadProject();

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
    void onObjectPropertyChanged();
    void showEstimate();
    void undo();
    void redo();
    void saveStateToUndoStack();
    void onShow3DClicked();
    void onDeleteLayerClicked();
    void onDeleteFloorClicked();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::EditorWindow *ui;
    int m_projectId;
    EditorScene *m_scene;
    BaseEditorItem *m_trackedItem = nullptr;
    QLabel *m_coordLabel;
    QStandardItemModel *m_treeModel;
    QStandardItem* getOrCreateLayerNode(int levelId, const QString &layerName);
    void loadMaterialsForComponent(class QComboBox *comboBox, const QString &systemCode);
    void initMaterialComboBoxes();
    bool m_hasUnsavedChanges = false;
    void setUnsavedChanges(bool changed);
    QList<QByteArray> m_undoStack;
    int m_undoIndex = -1;
    bool m_isRestoringState = false;
    QTimer *m_undoTimer;

    QByteArray captureSceneState();
    void restoreSceneState(const QByteArray &stateData);
};

Q_DECLARE_METATYPE(BaseEditorItem*)

#endif
