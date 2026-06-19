#include "editorwindow.h"
#include "ui_editorwindow.h"
#include "foundationblockitem.h"
#include "wallitem.h"
#include "windowitem.h"
#include "dooritem.h"
#include "nodeitem.h"
#include "flooritem.h"
#include "dimensionitem.h"
#include "textitem.h"
#include <QInputDialog>
#include "roofitem.h"
#include "objectitem.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QDir>
#include "filestoragemanager.h"
#include <QMessageBox>
#include <QButtonGroup>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "estimatedialog.h"
#include "viewer3d.h"

EditorWindow::EditorWindow(int projectId, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow),
    m_projectId(projectId)
{
    ui->setupUi(this);

    initMaterialComboBoxes();

    setWindowTitle(QString("Визуальный редактор — Проект №%1").arg(m_projectId));
    setMinimumSize(1000, 700);

    m_scene = new EditorScene(this);
    ui->graphicsView->setScene(m_scene);
    ui->graphicsView->centerOn(0, 0);

    ui->stackedWidget->setCurrentIndex(0);

    QButtonGroup *toolButtonGroup = new QButtonGroup(this);
    toolButtonGroup->setExclusive(true);

    toolButtonGroup->addButton(ui->pb_tool_cursor);
    toolButtonGroup->addButton(ui->pb_tool_foundation);
    toolButtonGroup->addButton(ui->pb_tool_wall);
    toolButtonGroup->addButton(ui->pb_tool_node);
    toolButtonGroup->addButton(ui->pb_tool_window);
    toolButtonGroup->addButton(ui->pb_tool_door);
    toolButtonGroup->addButton(ui->pb_tool_floor);
    toolButtonGroup->addButton(ui->pb_tool_dimension);
    toolButtonGroup->addButton(ui->pb_tool_text);
    toolButtonGroup->addButton(ui->pb_tool_roof);
    toolButtonGroup->addButton(ui->pb_tool_object);

    connect(ui->pb_tool_cursor, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_foundation, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_wall, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_node, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_window, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_door, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_floor, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_dimension, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_text, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_roof, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);

    connect(m_scene, &QGraphicsScene::selectionChanged, this, &EditorWindow::onSelectionChanged);

    connect(ui->sb_found_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onFoundationPropertyChanged);
    connect(ui->sb_found_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onFoundationPropertyChanged);
    connect(ui->sb_object_found_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onFoundationPropertyChanged);
    connect(ui->sb_found_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onFoundationPropertyChanged);

    connect(ui->sb_wall_length, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onWallPropertyChanged);
    connect(ui->sb_wall_thickness, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onWallPropertyChanged);
    connect(ui->sb_wall_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onWallPropertyChanged);
    connect(ui->sb_wall_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onWallPropertyChanged);
    connect(ui->cb_wall_alignment, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onWallPropertyChanged);

    connect(ui->sb_window_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onWindowPropertyChanged);
    connect(ui->sb_window_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onWindowPropertyChanged);
    connect(ui->sb_window_elevation, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onWindowPropertyChanged);
    connect(ui->sb_window_distance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onWindowPropertyChanged);

    connect(ui->sb_door_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onDoorPropertyChanged);
    connect(ui->sb_door_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onDoorPropertyChanged);
    connect(ui->sb_door_distance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onDoorPropertyChanged);
    connect(ui->cb_door_swing, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onDoorPropertyChanged);

    connect(ui->sb_floor_thickness, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onFloorPropertyChanged);

    connect(ui->sb_node_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onNodePropertyChanged);
    connect(ui->sb_node_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onNodePropertyChanged);
    connect(ui->sb_node_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onNodePropertyChanged);
    connect(ui->cb_node_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onNodePropertyChanged);

    connect(ui->cb_dim_side, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onDimensionPropertyChanged);
    connect(ui->le_text_content, &QLineEdit::textEdited, this, &EditorWindow::onTextPropertyChanged);
    connect(ui->sb_text_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onTextPropertyChanged);

    connect(ui->cb_roof_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onRoofPropertyChanged);
    connect(ui->sb_roof_overhang, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onRoofPropertyChanged);
    connect(ui->sb_roof_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onRoofPropertyChanged);
    connect(ui->sb_roof_thickness, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onRoofPropertyChanged);

    connect(ui->le_object_foundation_name, &QLineEdit::textEdited, this, [this](const QString &text) { if (m_trackedItem) m_trackedItem->setName(text); });
    connect(ui->le_object_wall_name, &QLineEdit::textEdited, this, [this](const QString &text) { if (m_trackedItem) m_trackedItem->setName(text); });
    connect(ui->le_object_window_name, &QLineEdit::textEdited, this, [this](const QString &text) { if (m_trackedItem) m_trackedItem->setName(text); });
    connect(ui->le_object_door_name, &QLineEdit::textEdited, this, [this](const QString &text) { if (m_trackedItem) m_trackedItem->setName(text); });
    connect(ui->le_object_node_name, &QLineEdit::textEdited, this, [this](const QString &text) { if (m_trackedItem) m_trackedItem->setName(text); });
    connect(ui->le_object_floor_name, &QLineEdit::textEdited, this, [this](const QString &text) { if (m_trackedItem) m_trackedItem->setName(text); });
    connect(ui->le_object_line_name, &QLineEdit::textEdited, this, [this](const QString &text) { if (m_trackedItem) m_trackedItem->setName(text); });
    connect(ui->le_object_text_name, &QLineEdit::textEdited, this, [this](const QString &text) { if (m_trackedItem) m_trackedItem->setName(text); });
    connect(ui->le_object_roof_name, &QLineEdit::textEdited, this, [this](const QString &text) { if (m_trackedItem) m_trackedItem->setName(text); });

    m_coordLabel = new QLabel("Клетка X: 0 | Y: 0", this);
    ui->statusBar->addPermanentWidget(m_coordLabel);

    connect(m_scene, &EditorScene::cursorMoved, this, [this](const QPointF &pos) {
        if (m_coordLabel) {
            m_coordLabel->setText(QString("X: %1   Y: %2").arg(qRound(pos.x())).arg(qRound(pos.y())));
        }
    });

    connect(m_scene, &EditorScene::toolModeChanged, this, [this](ToolMode mode) {
        ui->pb_tool_cursor->setChecked(mode == ModeCursor);
        ui->pb_tool_foundation->setChecked(mode == ModeFoundation);
        ui->pb_tool_wall->setChecked(mode == ModeWall);
        ui->pb_tool_node->setChecked(mode == ModeNode);
        ui->pb_tool_window->setChecked(mode == ModeWindow);
        ui->pb_tool_door->setChecked(mode == ModeDoor);
        ui->pb_tool_floor->setChecked(mode == ModeFloor);
        ui->pb_tool_dimension->setChecked(mode == ModeDimension);
        ui->pb_tool_text->setChecked(mode == ModeText);
        ui->pb_tool_roof->setChecked(mode == ModeRoof);
        ui->pb_tool_object->setChecked(mode == ModeObject);
    });

    connect(ui->sb_workspace_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
        m_scene->setWorkspaceSize(val, ui->sb_workspace_height->value());
        setUnsavedChanges(true);
    });
    connect(ui->sb_workspace_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
        m_scene->setWorkspaceSize(ui->sb_workspace_width->value(), val);
        setUnsavedChanges(true);
    });

    m_treeModel = new QStandardItemModel(this);
    m_treeModel->setHorizontalHeaderLabels(QStringList() << "Элемент" << "Слой" << "Этаж");
    ui->objectTreeView->setModel(m_treeModel);
    ui->objectTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->objectTreeView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(m_scene, &EditorScene::itemAdded, this, &EditorWindow::onItemAddedToScene);
    connect(ui->objectTreeView, &QTreeView::clicked, this, &EditorWindow::onTreeItemClicked);

    connect(ui->cb_active_floor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        QString text = ui->cb_active_floor->currentText();
        text.remove("Этаж ");
        if (m_scene) m_scene->setActiveLevel(text.toInt());
    });

    connect(ui->cb_active_layer, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
        if (m_scene) m_scene->setActiveLayer(ui->cb_active_layer->currentText());
    });

    connect(ui->btn_add_floor, &QPushButton::clicked, this, [this]() {
        int currentCount = ui->cb_active_floor->count();
        int newLevelId = currentCount + 1;
        ui->cb_active_floor->addItem(QString("Этаж %1").arg(newLevelId));
        ui->cb_active_floor->setCurrentIndex(currentCount);
    });

    connect(ui->btn_add_layer, &QPushButton::clicked, this, [this]() {
        bool ok;
        QString newLayerName = QInputDialog::getText(this, "Новый слой", "Введите название слоя:", QLineEdit::Normal, "", &ok);
        if (ok && !newLayerName.isEmpty()) {
            if (ui->cb_active_layer->findText(newLayerName) == -1) {
                ui->cb_active_layer->addItem(newLayerName);
            }
            ui->cb_active_layer->setCurrentText(newLayerName);
        }
    });

    connect(m_treeModel, &QStandardItemModel::itemChanged, this, [this](QStandardItem *item) {
        if (item->isCheckable()) {
            bool isVisible = (item->checkState() == Qt::Checked);
            QString layerName = item->text();
            if (m_scene) m_scene->setLayerVisible(layerName, isVisible);
        }
    });

    connect(ui->le_object_name, &QLineEdit::textEdited, this, &EditorWindow::onObjectPropertyChanged);
    connect(ui->sb_object_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onObjectPropertyChanged);
    connect(ui->sb_object_length, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onObjectPropertyChanged);
    connect(ui->sb_object_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onObjectPropertyChanged);
    connect(ui->pb_tool_object, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);

    connect(ui->action_saveProject, &QAction::triggered, this, &EditorWindow::saveProject);

    connect(ui->cb_foundation_material, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onFoundationPropertyChanged);
    connect(ui->cb_wall_material, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onWallPropertyChanged);
    connect(ui->cb_window_material, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onWindowPropertyChanged);
    connect(ui->cb_door_material, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onDoorPropertyChanged);
    connect(ui->cb_node_material, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onNodePropertyChanged);
    connect(ui->cb_floor_material, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onFloorPropertyChanged);
    connect(ui->cb_roof_material, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onRoofPropertyChanged);
    connect(ui->cb_furniture_material, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onObjectPropertyChanged);

    connect(ui->action_calculate_estimate, &QAction::triggered, this, &EditorWindow::showEstimate);

    connect(m_scene, &EditorScene::itemAdded, this, &EditorWindow::onItemAddedToScene);
    connect(m_scene, &EditorScene::itemDeleted, this, [this]() {
        setUnsavedChanges(true);
        m_undoTimer->stop();
        saveStateToUndoStack();
    });

    m_undoTimer = new QTimer(this);
    m_undoTimer->setSingleShot(true);
    m_undoTimer->setInterval(300);
    connect(m_undoTimer, &QTimer::timeout, this, &EditorWindow::saveStateToUndoStack);

    ui->action_undo->setShortcuts(QKeySequence::Undo);
    ui->action_redo->setShortcuts(QKeySequence::Redo);

    connect(ui->action_undo, &QAction::triggered, this, &EditorWindow::undo);
    connect(ui->action_redo, &QAction::triggered, this, &EditorWindow::redo);

    connect(ui->action_3D, &QAction::triggered, this, &EditorWindow::onShow3DClicked);

    loadProject();
}

EditorWindow::~EditorWindow()
{
    if (m_scene) m_scene->disconnect(this);
    delete ui;
}

void EditorWindow::onToolButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    if (button == ui->pb_tool_cursor) m_scene->setToolMode(ModeCursor);
    else if (button == ui->pb_tool_foundation) m_scene->setToolMode(ModeFoundation);
    else if (button == ui->pb_tool_wall) m_scene->setToolMode(ModeWall);
    else if (button == ui->pb_tool_node) m_scene->setToolMode(ModeNode);
    else if (button == ui->pb_tool_window) m_scene->setToolMode(ModeWindow);
    else if (button == ui->pb_tool_door) m_scene->setToolMode(ModeDoor);
    else if (button == ui->pb_tool_floor) m_scene->setToolMode(ModeFloor);
    else if (button == ui->pb_tool_dimension) m_scene->setToolMode(ModeDimension);
    else if (button == ui->pb_tool_text) m_scene->setToolMode(ModeText);
    else if (button == ui->pb_tool_roof) m_scene->setToolMode(ModeRoof);
    else if (button == ui->pb_tool_object) m_scene->setToolMode(ModeObject);
}

void EditorWindow::onSelectionChanged()
{
    if (m_trackedItem) {
        disconnect(m_trackedItem, &BaseEditorItem::itemChanged, this, nullptr);
        m_trackedItem = nullptr;
    }

    QList<QList<QGraphicsItem*>::value_type> selected = m_scene->selectedItems();

    if (selected.isEmpty() || selected.size() > 1) {
        ui->stackedWidget->setCurrentIndex(0);

        ui->sb_workspace_width->blockSignals(true);
        ui->sb_workspace_height->blockSignals(true);
        ui->sb_workspace_width->setValue(m_scene->workspaceSize().width());
        ui->sb_workspace_height->setValue(m_scene->workspaceSize().height());
        ui->sb_workspace_width->blockSignals(false);
        ui->sb_workspace_height->blockSignals(false);
        return;
    }

    QGraphicsItem *item = selected.first();
    m_trackedItem = dynamic_cast<BaseEditorItem*>(item);

    if (!m_trackedItem) return;

    if (FoundationBlockItem *block = dynamic_cast<FoundationBlockItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(1);

        auto updateUI = [this, block]() {
            ui->le_object_foundation_name->blockSignals(true);
            ui->sb_found_width->blockSignals(true);
            ui->sb_found_height->blockSignals(true);
            ui->sb_object_found_height->blockSignals(true);
            ui->sb_found_angle->blockSignals(true);
            ui->cb_foundation_material->blockSignals(true);

            ui->le_object_foundation_name->setText(block->name());
            ui->sb_found_width->setValue(block->widthInMeters());
            ui->sb_found_height->setValue(block->lengthInMeters());
            ui->sb_object_found_height->setValue(block->height());
            ui->sb_found_angle->setValue(block->rotation());

            int matIdx = ui->cb_foundation_material->findData(block->materialId());
            ui->cb_foundation_material->setCurrentIndex(matIdx != -1 ? matIdx : 0);

            ui->lbl_found_area->setText(QString("Площадь: %1 м² | Объем бетона: %2 м³")
                                            .arg(block->area(), 0, 'f', 2)
                                            .arg(block->volume(), 0, 'f', 2));

            ui->le_object_foundation_name->blockSignals(false);
            ui->sb_found_width->blockSignals(false);
            ui->sb_found_height->blockSignals(false);
            ui->sb_object_found_height->blockSignals(false);
            ui->sb_found_angle->blockSignals(false);
            ui->cb_foundation_material->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (WallItem *wall = dynamic_cast<WallItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(2);

        auto updateUI = [this, wall]() {
            ui->le_object_wall_name->blockSignals(true);
            ui->sb_wall_length->blockSignals(true);
            ui->sb_wall_thickness->blockSignals(true);
            ui->sb_wall_height->blockSignals(true);
            ui->sb_wall_angle->blockSignals(true);
            ui->cb_wall_alignment->blockSignals(true);
            ui->cb_wall_material->blockSignals(true);

            ui->le_object_wall_name->setText(wall->name());
            ui->sb_wall_length->setValue(wall->lengthInMeters());
            ui->sb_wall_thickness->setValue(wall->thicknessInMm());
            ui->sb_wall_height->setValue(wall->height());
            ui->sb_wall_angle->setValue(wall->angleInDegrees());
            ui->cb_wall_alignment->setCurrentIndex(wall->alignment());

            int matIdx = ui->cb_wall_material->findData(wall->materialId());
            ui->cb_wall_material->setCurrentIndex(matIdx != -1 ? matIdx : 0);

            ui->lbl_wall_area->setText(QString("Площадь: %1 м² | Объем: %2 м³")
                                           .arg(wall->netSurfaceArea(), 0, 'f', 2)
                                           .arg(wall->netVolume(), 0, 'f', 2));

            ui->le_object_wall_name->blockSignals(false);
            ui->sb_wall_length->blockSignals(false);
            ui->sb_wall_thickness->blockSignals(false);
            ui->sb_wall_height->blockSignals(false);
            ui->sb_wall_angle->blockSignals(false);
            ui->cb_wall_alignment->blockSignals(false);
            ui->cb_wall_material->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (WindowItem *window = dynamic_cast<WindowItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(3);

        auto updateUI = [this, window]() {
            ui->le_object_window_name->blockSignals(true);
            ui->sb_window_width->blockSignals(true);
            ui->sb_window_height->blockSignals(true);
            ui->sb_window_elevation->blockSignals(true);
            ui->sb_window_distance->blockSignals(true);
            ui->cb_window_material->blockSignals(true);

            ui->le_object_window_name->setText(window->name());
            ui->sb_window_width->setValue(window->widthInMeters());
            ui->sb_window_height->setValue(window->height());
            ui->sb_window_elevation->setValue(window->elevation());
            ui->sb_window_distance->setValue(window->distanceFromStart());

            int matIdx = ui->cb_window_material->findData(window->materialId());
            ui->cb_window_material->setCurrentIndex(matIdx != -1 ? matIdx : 0);

            ui->lbl_window_area->setText(QString("Площадь проема: %1 м²").arg(window->area(), 0, 'f', 2));

            ui->le_object_window_name->blockSignals(false);
            ui->sb_window_width->blockSignals(false);
            ui->sb_window_height->blockSignals(false);
            ui->sb_window_elevation->blockSignals(false);
            ui->sb_window_distance->blockSignals(false);
            ui->cb_window_material->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (DoorItem *door = dynamic_cast<DoorItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(4);

        auto updateUI = [this, door]() {
            ui->le_object_door_name->blockSignals(true);
            ui->sb_door_width->blockSignals(true);
            ui->sb_door_height->blockSignals(true);
            ui->sb_door_distance->blockSignals(true);
            ui->cb_door_swing->blockSignals(true);
            ui->cb_door_material->blockSignals(true);

            ui->le_object_door_name->setText(door->name());
            ui->sb_door_width->setValue(door->widthInMeters());
            ui->sb_door_height->setValue(door->height());
            ui->sb_door_distance->setValue(door->distanceFromStart());
            ui->cb_door_swing->setCurrentIndex(door->swingType());

            int matIdx = ui->cb_door_material->findData(door->materialId());
            ui->cb_door_material->setCurrentIndex(matIdx != -1 ? matIdx : 0);

            ui->lbl_door_area->setText(QString("Площадь проема: %1 м²").arg(door->area(), 0, 'f', 2));

            ui->le_object_door_name->blockSignals(false);
            ui->sb_door_width->blockSignals(false);
            ui->sb_door_height->blockSignals(false);
            ui->sb_door_distance->blockSignals(false);
            ui->cb_door_swing->blockSignals(false);
            ui->cb_door_material->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (NodeItem *node = dynamic_cast<NodeItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(5);

        auto updateUI = [this, node]() {
            ui->le_object_node_name->blockSignals(true);
            ui->sb_node_width->blockSignals(true);
            ui->sb_node_height->blockSignals(true);
            ui->sb_node_angle->blockSignals(true);
            ui->cb_node_type->blockSignals(true);

            ui->le_object_node_name->setText(node->name());
            ui->sb_node_width->setValue(node->side1InMeters());
            ui->sb_node_height->setValue(node->side2InMeters());
            ui->sb_node_angle->setValue(node->rotationAngle());
            ui->cb_node_type->setCurrentIndex(node->isRounded());
            ui->lbl_node_area->setText(QString("Площадь: %1 м²").arg(node->area(), 0, 'f', 4));

            ui->lbl_node_height->setText(QString("%1 м").arg(node->maxAttachedWallHeight(), 0, 'f', 2));

            ui->le_object_node_name->blockSignals(false);
            ui->sb_node_width->blockSignals(false);
            ui->sb_node_height->blockSignals(false);
            ui->sb_node_angle->blockSignals(false);
            ui->cb_node_type->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (FloorItem *floor = dynamic_cast<FloorItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(6);

        auto updateUI = [this, floor]() {
            ui->le_object_floor_name->blockSignals(true);
            ui->sb_floor_thickness->blockSignals(true);
            ui->cb_floor_material->blockSignals(true);

            ui->le_object_floor_name->setText(floor->name());
            ui->sb_floor_thickness->setValue(floor->height());

            int matIdx = ui->cb_floor_material->findData(floor->materialId());
            ui->cb_floor_material->setCurrentIndex(matIdx != -1 ? matIdx : 0);

            ui->lbl_floor_area->setText(QString("Площадь: %1 м² | Объем бетона: %2 м³")
                                            .arg(floor->area(), 0, 'f', 2)
                                            .arg(floor->volume(), 0, 'f', 2));

            ui->le_object_floor_name->blockSignals(false);
            ui->sb_floor_thickness->blockSignals(false);
            ui->cb_floor_material->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (DimensionItem *dim = dynamic_cast<DimensionItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(7);

        auto updateUI = [this, dim]() {
            ui->le_object_line_name->blockSignals(true);
            ui->cb_dim_side->blockSignals(true);

            ui->le_object_line_name->setText(dim->name());
            ui->lbl_dim_length->setText(QString("Длина: %1 мм").arg(dim->lengthInMeters() * 1000.0, 0, 'f', 0));
            ui->cb_dim_side->setCurrentIndex(dim->textSide());

            ui->le_object_line_name->blockSignals(false);
            ui->cb_dim_side->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (TextItem *textItem = dynamic_cast<TextItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(8);

        auto updateUI = [this, textItem]() {
            ui->le_object_text_name->blockSignals(true);
            ui->le_text_content->blockSignals(true);
            ui->sb_text_size->blockSignals(true);
            ui->sb_text_angle->blockSignals(true);

            ui->le_object_text_name->setText(textItem->name());
            ui->le_text_content->setText(textItem->textContent());
            ui->sb_text_size->setValue(textItem->fontSize());
            ui->sb_text_angle->setValue(textItem->rotationAngle());

            ui->le_object_text_name->blockSignals(false);
            ui->le_text_content->blockSignals(false);
            ui->sb_text_size->blockSignals(false);
            ui->sb_text_angle->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (RoofItem *roof = dynamic_cast<RoofItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(9);

        auto updateUI = [this, roof]() {
            ui->le_object_roof_name->blockSignals(true);
            ui->cb_roof_type->blockSignals(true);
            ui->sb_roof_overhang->blockSignals(true);
            ui->sb_roof_angle->blockSignals(true);
            ui->sb_roof_thickness->blockSignals(true);
            ui->cb_roof_material->blockSignals(true);

            ui->le_object_roof_name->setText(roof->name());
            ui->cb_roof_type->setCurrentIndex(roof->roofType());
            ui->sb_roof_overhang->setValue(roof->overhang());
            ui->sb_roof_thickness->setValue(roof->height());

            int matIdx = ui->cb_roof_material->findData(roof->materialId());
            ui->cb_roof_material->setCurrentIndex(matIdx != -1 ? matIdx : 0);

            if (roof->roofType() == RoofItem::Flat) {
                ui->sb_roof_angle->setValue(0);
                ui->sb_roof_angle->setEnabled(false);
            } else {
                ui->sb_roof_angle->setValue(roof->angle());
                ui->sb_roof_angle->setEnabled(true);
            }

            ui->lbl_roof_area->setText(QString("Площадь: %1 м² | Объем: %2 м³")
                                           .arg(roof->area(), 0, 'f', 2)
                                           .arg(roof->volume(), 0, 'f', 2));

            ui->le_object_roof_name->blockSignals(false);
            ui->cb_roof_type->blockSignals(false);
            ui->sb_roof_overhang->blockSignals(false);
            ui->sb_roof_angle->blockSignals(false);
            ui->sb_roof_thickness->blockSignals(false);
            ui->cb_roof_material->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (ObjectItem *obj = dynamic_cast<ObjectItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(10);

        auto updateUI = [this, obj]() {
            ui->le_object_name->blockSignals(true);
            ui->sb_object_width->blockSignals(true);
            ui->sb_object_length->blockSignals(true);
            ui->sb_object_angle->blockSignals(true);
            ui->cb_furniture_material->blockSignals(true);

            ui->le_object_name->setText(obj->name());
            ui->sb_object_width->setValue(obj->widthInMeters());
            ui->sb_object_length->setValue(obj->lengthInMeters());
            ui->sb_object_angle->setValue(obj->rotation());

            int matIdx = ui->cb_furniture_material->findData(obj->materialId());
            ui->cb_furniture_material->setCurrentIndex(matIdx != -1 ? matIdx : 0);

            ui->le_object_name->blockSignals(false);
            ui->sb_object_width->blockSignals(false);
            ui->sb_object_length->blockSignals(false);
            ui->sb_object_angle->blockSignals(false);
            ui->cb_furniture_material->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
}

void EditorWindow::onFoundationPropertyChanged()
{
    if (m_trackedItem) {
        if (FoundationBlockItem *block = dynamic_cast<FoundationBlockItem*>(m_trackedItem)) {
            block->setMaterialId(ui->cb_foundation_material->currentData().toInt());

            QString newName = ui->le_object_foundation_name->text();
            if (block->name() != newName) block->setName(newName);

            block->setWidthInMeters(ui->sb_found_width->value());
            block->setLengthInMeters(ui->sb_found_height->value());
            block->setHeight(ui->sb_object_found_height->value());
            block->setRotation(ui->sb_found_angle->value());
        }
    }
}

void EditorWindow::onWallPropertyChanged()
{
    if (m_trackedItem) {
        if (WallItem *wall = dynamic_cast<WallItem*>(m_trackedItem)) {
            wall->setMaterialId(ui->cb_wall_material->currentData().toInt());

            QString newName = ui->le_object_wall_name->text();
            if (wall->name() != newName) wall->setName(newName);

            wall->setLengthInMeters(ui->sb_wall_length->value());
            wall->setThicknessInMm(ui->sb_wall_thickness->value());
            wall->setHeight(ui->sb_wall_height->value());
            wall->setAngleInDegrees(ui->sb_wall_angle->value());
            wall->setAlignment(ui->cb_wall_alignment->currentIndex());
        }
    }
}

void EditorWindow::onWindowPropertyChanged()
{
    if (m_trackedItem) {
        if (WindowItem *window = dynamic_cast<WindowItem*>(m_trackedItem)) {
            window->setMaterialId(ui->cb_window_material->currentData().toInt());

            QString newName = ui->le_object_window_name->text();
            if (window->name() != newName) window->setName(newName);

            window->setWidthInMeters(ui->sb_window_width->value());
            window->setHeight(ui->sb_window_height->value());
            window->setElevation(ui->sb_window_elevation->value());
            window->setDistanceFromStart(ui->sb_window_distance->value());
        }
    }
}

void EditorWindow::onDoorPropertyChanged()
{
    if (m_trackedItem) {
        if (DoorItem *door = dynamic_cast<DoorItem*>(m_trackedItem)) {
            door->setMaterialId(ui->cb_door_material->currentData().toInt());

            QString newName = ui->le_object_door_name->text();
            if (door->name() != newName) door->setName(newName);

            door->setWidthInMeters(ui->sb_door_width->value());
            door->setHeight(ui->sb_door_height->value());
            door->setDistanceFromStart(ui->sb_door_distance->value());
            door->setSwingType(ui->cb_door_swing->currentIndex());
        }
    }
}

void EditorWindow::onNodePropertyChanged()
{
    if (m_trackedItem) {
        if (NodeItem *node = dynamic_cast<NodeItem*>(m_trackedItem)) {
            node->setMaterialId(ui->cb_node_material->currentData().toInt());

            QString newName = ui->le_object_node_name->text();
            if (node->name() != newName) node->setName(newName);

            node->setSide1InMeters(ui->sb_node_width->value());
            node->setSide2InMeters(ui->sb_node_height->value());
            node->setRotationAngle(ui->sb_node_angle->value());
            node->setRounded(ui->cb_node_type->currentIndex());
        }
    }
}

void EditorWindow::onDimensionPropertyChanged()
{
    if (m_trackedItem) {
        if (DimensionItem *dim = dynamic_cast<DimensionItem*>(m_trackedItem)) {
            QString newName = ui->le_object_line_name->text();
            if (dim->name() != newName) dim->setName(newName);

            dim->setTextSide(ui->cb_dim_side->currentIndex());
        }
    }
}

void EditorWindow::onFloorPropertyChanged()
{
    if (m_trackedItem) {
        if (FloorItem *floor = dynamic_cast<FloorItem*>(m_trackedItem)) {
            floor->setMaterialId(ui->cb_floor_material->currentData().toInt());

            QString newName = ui->le_object_floor_name->text();
            if (floor->name() != newName) floor->setName(newName);

            floor->setHeight(ui->sb_floor_thickness->value());
        }
    }
}

void EditorWindow::onTextPropertyChanged()
{
    if (m_trackedItem) {
        if (TextItem *textItem = dynamic_cast<TextItem*>(m_trackedItem)) {
            QString newName = ui->le_object_text_name->text();
            if (textItem->name() != newName) textItem->setName(newName);

            textItem->setTextContent(ui->le_text_content->text());
            textItem->setFontSize(ui->sb_text_size->value());
            textItem->setRotationAngle(ui->sb_text_angle->value());
        }
    }
}

void EditorWindow::onRoofPropertyChanged()
{
    if (m_trackedItem) {
        if (RoofItem *roof = dynamic_cast<RoofItem*>(m_trackedItem)) {
            roof->setMaterialId(ui->cb_roof_material->currentData().toInt());

            QString newName = ui->le_object_roof_name->text();
            if (roof->name() != newName) roof->setName(newName);

            int newType = ui->cb_roof_type->currentIndex();
            double newAngle = ui->sb_roof_angle->value();

            if (newType == RoofItem::Flat) {
                newAngle = 0.0;
                ui->sb_roof_angle->blockSignals(true);
                ui->sb_roof_angle->setValue(0);
                ui->sb_roof_angle->setEnabled(false);
                ui->sb_roof_angle->blockSignals(false);
            } else {
                ui->sb_roof_angle->setEnabled(true);
            }

            roof->setRoofType(newType);
            roof->setOverhang(ui->sb_roof_overhang->value());
            roof->setAngle(newAngle);
            roof->setHeight(ui->sb_roof_thickness->value());
        }
    }
}

void EditorWindow::onItemAddedToScene(BaseEditorItem *item)
{
    if (!item) return;

    QList<QStandardItem*> rowItems;

    QStandardItem *nameItem = new QStandardItem(item->name().isEmpty() ? "Без имени" : item->name());
    nameItem->setData(QVariant::fromValue(item), Qt::UserRole + 1);

    QStandardItem *layerItem = new QStandardItem(item->layerName());
    QStandardItem *levelItem = new QStandardItem(QString::number(item->levelId()));

    rowItems << nameItem << layerItem << levelItem;
    QStandardItem *parentLayerNode = getOrCreateLayerNode(item->levelId(), item->layerName());
    parentLayerNode->appendRow(rowItems);

    connect(item, &BaseEditorItem::itemChanged, this, &EditorWindow::onItemChangedInScene);
    ui->objectTreeView->expandAll();

    setUnsavedChanges(true);

    m_undoTimer->stop();
    saveStateToUndoStack();
}

void EditorWindow::onItemChangedInScene()
{
    BaseEditorItem *item = qobject_cast<BaseEditorItem*>(sender());
    if (!item) return;

    QModelIndexList matches = m_treeModel->match(
        m_treeModel->index(0, 0),
        Qt::UserRole + 1,
        QVariant::fromValue(item),
        1,
        Qt::MatchExactly | Qt::MatchRecursive
        );

    if (!matches.isEmpty()) {
        QStandardItem *nameItem = m_treeModel->itemFromIndex(matches.first());
        if (nameItem) {
            QString currentName = item->name().isEmpty() ? "Без имени" : item->name();
            if (nameItem->text() != currentName) nameItem->setText(currentName);

            QStandardItem *parentRow = nameItem->parent();
            int rowIdx = nameItem->row();

            if (parentRow) {
                QStandardItem *layerItem = parentRow->child(rowIdx, 1);
                if (layerItem && layerItem->text() != item->layerName()) {
                    layerItem->setText(item->layerName());
                }

                QStandardItem *levelItem = parentRow->child(rowIdx, 2);
                if (levelItem && levelItem->text() != QString::number(item->levelId())) {
                    levelItem->setText(QString::number(item->levelId()));
                }
            }
        }
    }

    setUnsavedChanges(true);
}

void EditorWindow::onTreeItemClicked(const QModelIndex &index)
{
    QModelIndex nameIndex = m_treeModel->index(index.row(), 0);

    QVariant var = m_treeModel->data(nameIndex, Qt::UserRole + 1);
    if (var.canConvert<BaseEditorItem*>()) {
        BaseEditorItem *item = var.value<BaseEditorItem*>();
        if (item && item->scene()) {
            item->scene()->clearSelection();
            item->setSelected(true);
        }
    }
}

QStandardItem* EditorWindow::getOrCreateLayerNode(int levelId, const QString &layerName)
{
    QString levelStr = QString("Этаж %1").arg(levelId);

    QStandardItem *levelNode = nullptr;
    for (int i = 0; i < m_treeModel->rowCount(); ++i) {
        if (m_treeModel->item(i, 0)->text() == levelStr) {
            levelNode = m_treeModel->item(i, 0);
            break;
        }
    }
    if (!levelNode) {
        levelNode = new QStandardItem(levelStr);
        m_treeModel->appendRow(QList<QStandardItem*>() << levelNode << new QStandardItem("") << new QStandardItem(""));
    }

    QStandardItem *layerNode = nullptr;
    for (int i = 0; i < levelNode->rowCount(); ++i) {
        if (levelNode->child(i, 0)->text() == layerName) {
            layerNode = levelNode->child(i, 0);
            break;
        }
    }
    if (!layerNode) {
        layerNode = new QStandardItem(layerName);

        layerNode->setCheckable(true);
        layerNode->setCheckState(Qt::Checked);

        levelNode->appendRow(QList<QStandardItem*>() << layerNode << new QStandardItem("") << new QStandardItem(""));
    }

    return layerNode;
}

void EditorWindow::onObjectPropertyChanged()
{
    if (m_trackedItem) {
        if (ObjectItem *obj = dynamic_cast<ObjectItem*>(m_trackedItem)) {
            int matId = ui->cb_furniture_material->currentData().toInt();
            obj->setMaterialId(matId);

            if (matId == -1) {
                obj->setMaterialName("");
            } else {
                obj->setMaterialName(ui->cb_furniture_material->currentText());
            }

            QString newName = ui->le_object_name->text();
            if (obj->name() != newName) obj->setName(newName);

            obj->setWidthInMeters(ui->sb_object_width->value());
            obj->setLengthInMeters(ui->sb_object_length->value());

            double newAngle = ui->sb_object_angle->value();
            if (qAbs(obj->rotation() - newAngle) > 0.01) {
                obj->setRotation(newAngle);
            }
        }
    }
}

void EditorWindow::saveProject()
{
    QString folderPath = FileStorageManager::getProjectFolder(m_projectId);
    QString filePath = folderPath + "/layout.json";

    QByteArray compactData = captureSceneState();

    QJsonDocument doc = QJsonDocument::fromJson(compactData);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();

        setUnsavedChanges(false);
        QMessageBox::information(this, "Сохранение", "Проект успешно сохранен!");
    } else {
        QMessageBox::critical(this, "Ошибка", "Не удалось записать файл проекта.");
    }
}

void EditorWindow::loadProject()
{
    QString folderPath = FileStorageManager::getProjectFolder(m_projectId);
    QString filePath = folderPath + "/layout.json";

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_undoStack.clear();
        m_undoStack.append(captureSceneState());
        m_undoIndex = 0;
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    m_isRestoringState = true;
    restoreSceneState(data);
    setUnsavedChanges(false);

    m_undoStack.clear();
    m_undoStack.append(captureSceneState());
    m_undoIndex = 0;

    QTimer::singleShot(100, this, [this]() {
        m_isRestoringState = false;
    });
}

void EditorWindow::loadMaterialsForComponent(QComboBox *comboBox, const QString &systemCode)
{
    if (!comboBox) return;

    comboBox->blockSignals(true);
    comboBox->clear();

    comboBox->addItem("— Не выбран —", -1);

    QSqlQuery query;
    query.prepare("SELECT m.id, m.name "
                  "FROM materials m "
                  "JOIN categories c ON m.category_id = c.id "
                  "WHERE c.system_code = :code "
                  "ORDER BY m.name ASC");
    query.bindValue(":code", systemCode);

    if (query.exec()) {
        while (query.next()) {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();

            comboBox->addItem(name, id);
        }
    }

    comboBox->blockSignals(false);
}

void EditorWindow::initMaterialComboBoxes()
{
    loadMaterialsForComponent(ui->cb_foundation_material, "FOUNDATION_MAT");
    loadMaterialsForComponent(ui->cb_wall_material, "WALL_MAT");
    loadMaterialsForComponent(ui->cb_node_material, "WALL_MAT");
    loadMaterialsForComponent(ui->cb_floor_material, "FLOOR_MAT");
    loadMaterialsForComponent(ui->cb_window_material, "WINDOW_MAT");
    loadMaterialsForComponent(ui->cb_door_material, "DOOR_MAT");
    loadMaterialsForComponent(ui->cb_roof_material, "ROOF_MAT");
    loadMaterialsForComponent(ui->cb_furniture_material, "FURNITURE_MAT");
}

void EditorWindow::showEstimate()
{
    QList<BaseEditorItem*> editorItems;
    for (QGraphicsItem *gItem : m_scene->items()) {
        if (BaseEditorItem *bItem = dynamic_cast<BaseEditorItem*>(gItem)) {
            if (FloorItem *floor = dynamic_cast<FloorItem*>(bItem)) {
                if (floor->isDrawing()) continue;
            }
            if (RoofItem *roof = dynamic_cast<RoofItem*>(bItem)) {
                if (roof->isDrawing()) continue;
            }
            editorItems.append(bItem);
        }
    }

    EstimateDialog dialog(editorItems, this);
    dialog.exec();
}

void EditorWindow::setUnsavedChanges(bool changed)
{
    m_hasUnsavedChanges = changed;
    QString baseTitle = QString("Визуальный редактор — Проект №%1").arg(m_projectId);

    if (m_hasUnsavedChanges) {
        setWindowTitle(baseTitle + " *");
        if (!m_isRestoringState && m_undoTimer) {
            m_undoTimer->start();
        }
    } else {
        setWindowTitle(baseTitle);
    }
}

void EditorWindow::closeEvent(QCloseEvent *event)
{
    if (m_hasUnsavedChanges) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Сохранение");
        msgBox.setText("В проекте есть несохраненные изменения.\nСохранить их перед выходом?");
        msgBox.setIcon(QMessageBox::Question);

        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        msgBox.setButtonText(QMessageBox::Yes, "Сохранить");
        msgBox.setButtonText(QMessageBox::No, "Не сохранять");
        msgBox.setButtonText(QMessageBox::Cancel, "Отмена");

        msgBox.setDefaultButton(QMessageBox::Yes);

        int resBtn = msgBox.exec();

        if (resBtn == QMessageBox::Yes) {
            saveProject();
            if (!m_hasUnsavedChanges) {
                event->accept();
            } else {
                event->ignore();
            }
        } else if (resBtn == QMessageBox::No) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

QByteArray EditorWindow::captureSceneState()
{
    QJsonArray itemsArray;
    for (QGraphicsItem *gItem : m_scene->items()) {
        if (BaseEditorItem *item = dynamic_cast<BaseEditorItem*>(gItem)) {
            if (FloorItem *floor = dynamic_cast<FloorItem*>(item)) { if (floor->isDrawing()) continue; }
            if (RoofItem *roof = dynamic_cast<RoofItem*>(item)) { if (roof->isDrawing()) continue; }
            if (DimensionItem *dim = dynamic_cast<DimensionItem*>(item)) { if (dim->isDrawing()) continue; }
            itemsArray.append(item->toJson());
        }
    }

    QJsonObject rootObject;
    rootObject["version"] = "1.0";
    rootObject["items"] = itemsArray;
    rootObject["workspace_w"] = m_scene->sceneRect().width();
    rootObject["workspace_h"] = m_scene->sceneRect().height();

    return QJsonDocument(rootObject).toJson(QJsonDocument::Compact);
}

void EditorWindow::saveStateToUndoStack()
{
    if (m_isRestoringState) return;

    QByteArray currentState = captureSceneState();

    if (m_undoIndex >= 0 && m_undoIndex < m_undoStack.size()) {
        if (m_undoStack[m_undoIndex] == currentState) return;
    }

    if (m_undoIndex < m_undoStack.size() - 1) {
        m_undoStack.erase(m_undoStack.begin() + m_undoIndex + 1, m_undoStack.end());
    }

    m_undoStack.append(currentState);

    if (m_undoStack.size() > 50) {
        m_undoStack.removeFirst();
    } else {
        m_undoIndex++;
    }
}

void EditorWindow::undo()
{
    if (m_undoIndex > 0) {
        m_isRestoringState = true;
        m_undoTimer->stop();

        m_undoIndex--;
        restoreSceneState(m_undoStack[m_undoIndex]);
        setUnsavedChanges(true);

        QTimer::singleShot(100, this, [this]() {
            m_isRestoringState = false;
        });
    }
}

void EditorWindow::redo()
{
    if (m_undoIndex < m_undoStack.size() - 1) {
        m_isRestoringState = true;
        m_undoTimer->stop();

        m_undoIndex++;
        restoreSceneState(m_undoStack[m_undoIndex]);
        setUnsavedChanges(true);

        QTimer::singleShot(100, this, [this]() {
            m_isRestoringState = false;
        });
    }
}

void EditorWindow::restoreSceneState(const QByteArray &stateData)
{
    m_trackedItem = nullptr;
    ui->stackedWidget->setCurrentIndex(0);

    QJsonDocument doc = QJsonDocument::fromJson(stateData);
    if (!doc.isObject()) {
        return;
    }

    QJsonObject rootObject = doc.object();

    if (rootObject.contains("workspace_w") && rootObject.contains("workspace_h")) {
        m_scene->setSceneRect(0, 0, rootObject["workspace_w"].toDouble(), rootObject["workspace_h"].toDouble());
        ui->sb_workspace_width->blockSignals(true);
        ui->sb_workspace_height->blockSignals(true);
        ui->sb_workspace_width->setValue(m_scene->workspaceSize().width());
        ui->sb_workspace_height->setValue(m_scene->workspaceSize().height());
        ui->sb_workspace_width->blockSignals(false);
        ui->sb_workspace_height->blockSignals(false);
    }

    QJsonArray itemsArray = rootObject["items"].toArray();

    m_scene->clearSelection();
    for (QGraphicsItem *item : m_scene->items()) {
        m_scene->removeItem(item);
        delete item;
    }
    m_treeModel->removeRows(0, m_treeModel->rowCount());

    for (const QJsonValue &value : itemsArray) {
        QJsonObject itemObj = value.toObject();
        if (BaseEditorItem *newItem = BaseEditorItem::createFromJson(itemObj)) {
            m_scene->addItem(newItem);
            emit m_scene->itemAdded(newItem);
        }
    }

    QSet<int> uniqueLevels;
    QSet<QString> uniqueLayers;
    for (QGraphicsItem *item : m_scene->items()) {
        if (BaseEditorItem *bItem = dynamic_cast<BaseEditorItem*>(item)) {
            uniqueLevels.insert(bItem->levelId());
            uniqueLayers.insert(bItem->layerName());
        }
    }

    ui->cb_active_floor->blockSignals(true);
    ui->cb_active_floor->clear();
    QList<int> sortedLevels = uniqueLevels.values();
    std::sort(sortedLevels.begin(), sortedLevels.end());
    for (int level : sortedLevels) {
        ui->cb_active_floor->addItem(QString("Этаж %1").arg(level));
    }
    ui->cb_active_floor->setCurrentIndex(0);
    m_scene->setActiveLevel(sortedLevels.isEmpty() ? 1 : sortedLevels.first());
    ui->cb_active_floor->blockSignals(false);

    ui->cb_active_layer->blockSignals(true);
    ui->cb_active_layer->clear();
    for (const QString &layer : uniqueLayers) {
        ui->cb_active_layer->addItem(layer);
    }
    ui->cb_active_layer->setCurrentIndex(0);
    m_scene->setActiveLayer(uniqueLayers.isEmpty() ? "Основной" : *uniqueLayers.begin());
    ui->cb_active_layer->blockSignals(false);

    m_scene->updateItemsVisibility();

    for (const QJsonValue &value : itemsArray) {
        QJsonObject itemObj = value.toObject();
        QString hostWallName = itemObj["host_wall_name"].toString();
        QString itemName = itemObj["name"].toString();

        if (!hostWallName.isEmpty()) {
            WallItem *foundWall = nullptr;
            BaseEditorItem *targetItem = nullptr;

            for (QGraphicsItem *item : m_scene->items()) {
                if (WallItem *w = dynamic_cast<WallItem*>(item)) {
                    if (w->name() == hostWallName) foundWall = w;
                }
                if (BaseEditorItem *b = dynamic_cast<BaseEditorItem*>(item)) {
                    if (b->name() == itemName) targetItem = b;
                }
            }

            if (foundWall && targetItem) {
                if (WindowItem *win = dynamic_cast<WindowItem*>(targetItem)) {
                    win->setHostWall(foundWall);
                    win->updateGeometryToWall();
                } else if (DoorItem *door = dynamic_cast<DoorItem*>(targetItem)) {
                    door->setHostWall(foundWall);
                    door->updateGeometryToWall();
                }
            }
        }
    }
}

void EditorWindow::onShow3DClicked()
{
    saveProject();

    QString filePath = FileStorageManager::getProjectFolder(m_projectId) + "/layout.json";
    Viewer3D *viewer = new Viewer3D(filePath);
    viewer->show();
}
