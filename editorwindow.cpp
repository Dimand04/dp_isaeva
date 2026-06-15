#include "editorwindow.h"
#include "ui_editorwindow.h"
#include "foundationblockitem.h"
#include "wallitem.h"
#include "windowitem.h"
#include "dooritem.h"
#include "nodeitem.h"

EditorWindow::EditorWindow(int projectId, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::EditorWindow),
    m_projectId(projectId)
{
    ui->setupUi(this);

    setWindowTitle(QString("Визуальный редактор — Проект №%1").arg(m_projectId));
    setMinimumSize(1000, 700);

    m_scene = new EditorScene(this);
    ui->graphicsView->setScene(m_scene);

    ui->graphicsView->centerOn(0, 0);

    ui->stackedWidget->setCurrentIndex(0);

    ui->le_object_name->setVisible(false);

    connect(ui->pb_tool_cursor, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_foundation, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_wall, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_node, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_window, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_door, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_floor, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);

    connect(m_scene, &QGraphicsScene::selectionChanged, this, &EditorWindow::onSelectionChanged);

    connect(ui->sb_found_width, &QDoubleSpinBox::editingFinished, this, &EditorWindow::onFoundationPropertyChanged);
    connect(ui->sb_found_height, &QDoubleSpinBox::editingFinished, this, &EditorWindow::onFoundationPropertyChanged);
    connect(ui->sb_wall_length, &QDoubleSpinBox::editingFinished, this, &EditorWindow::onWallPropertyChanged);
    connect(ui->sb_wall_thickness, &QDoubleSpinBox::editingFinished, this, &EditorWindow::onWallPropertyChanged);

    connect(ui->sb_found_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EditorWindow::onFoundationPropertyChanged);

    connect(ui->sb_wall_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EditorWindow::onWallPropertyChanged);

    connect(ui->sb_window_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EditorWindow::onWindowPropertyChanged);

    connect(ui->sb_window_elevation, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EditorWindow::onWindowPropertyChanged);

    connect(ui->cb_window_profile, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EditorWindow::onWindowPropertyChanged);

    connect(ui->sb_window_distance, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EditorWindow::onWindowPropertyChanged);

    connect(ui->sb_door_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EditorWindow::onDoorPropertyChanged);

    connect(ui->sb_door_distance, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EditorWindow::onDoorPropertyChanged);

    connect(ui->cb_door_swing, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EditorWindow::onDoorPropertyChanged);

    connect(ui->cb_wall_alignment, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EditorWindow::onWallPropertyChanged);

    m_coordLabel = new QLabel("Клетка X: 0 | Y: 0", this);
    ui->statusBar->addPermanentWidget(m_coordLabel);

    connect(m_scene, &EditorScene::cursorMoved, this, [this](const QPointF &pos) {
        int cellX = qFloor(pos.x() / 20.0);
        int cellY = qFloor(pos.y() / 20.0);

        m_coordLabel->setText(QString("Клетка X: %1 | Y: %2").arg(cellX).arg(cellY));
    });

    connect(ui->sb_node_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onNodePropertyChanged);
    connect(ui->sb_node_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onNodePropertyChanged);
    connect(ui->sb_node_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onNodePropertyChanged);
    connect(ui->cb_node_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onNodePropertyChanged);

    connect(ui->le_object_name, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_trackedItem) {
            m_trackedItem->setName(text);
        }
    });

    connect(ui->sb_workspace_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
        m_scene->setWorkspaceSize(val, ui->sb_workspace_height->value());
    });

    connect(ui->sb_workspace_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
        m_scene->setWorkspaceSize(ui->sb_workspace_width->value(), val);
    });
}

EditorWindow::~EditorWindow()
{
    if (m_scene) {
        m_scene->disconnect(this);
    }

    delete ui;
}

void EditorWindow::onToolButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    if (button == ui->pb_tool_cursor) {
        m_scene->setToolMode(ModeCursor);
    } else if (button == ui->pb_tool_foundation) {
        m_scene->setToolMode(ModeFoundation);
    } else if (button == ui->pb_tool_wall) {
        m_scene->setToolMode(ModeWall);
    } else if (button == ui->pb_tool_node) {
        m_scene->setToolMode(ModeNode);
    } else if (button == ui->pb_tool_window) {
        m_scene->setToolMode(ModeWindow);
    } else if (button == ui->pb_tool_door) {
        m_scene->setToolMode(ModeDoor);
    } else if (button == ui->pb_tool_floor) {
        m_scene->setToolMode(ModeFloor);
    }
}

void EditorWindow::onSelectionChanged()
{
    if (m_trackedItem) {
        disconnect(m_trackedItem, &BaseEditorItem::itemChanged, this, nullptr);
        m_trackedItem = nullptr;
    }

    QList<QGraphicsItem*> selected = m_scene->selectedItems();

    if (selected.isEmpty() || selected.size() > 1) {
        ui->stackedWidget->setCurrentIndex(0);
        ui->le_object_name->clear();
        ui->le_object_name->setEnabled(false);
        ui->le_object_name->setVisible(false);

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

    if (m_trackedItem) {
        ui->le_object_name->setVisible(true);
        ui->le_object_name->setEnabled(true);
        ui->le_object_name->blockSignals(true);
        ui->le_object_name->setText(m_trackedItem->name());
        ui->le_object_name->blockSignals(false);
    }

    if (FoundationBlockItem *block = dynamic_cast<FoundationBlockItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(1);

        auto updateUI = [this, block]() {
            ui->sb_found_width->blockSignals(true);
            ui->sb_found_height->blockSignals(true);
            ui->sb_found_angle->blockSignals(true);

            ui->sb_found_width->setValue(block->widthInMeters());
            ui->sb_found_height->setValue(block->heightInMeters());
            ui->sb_found_angle->setValue(block->rotation());
            ui->lbl_found_area->setText(QString("Площадь: %1 м²").arg(block->area(), 0, 'f', 2));

            ui->sb_found_width->blockSignals(false);
            ui->sb_found_height->blockSignals(false);
            ui->sb_found_angle->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (WallItem *wall = dynamic_cast<WallItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(2);

        auto updateUI = [this, wall]() {
            ui->sb_wall_length->blockSignals(true);
            ui->sb_wall_thickness->blockSignals(true);
            ui->sb_wall_angle->blockSignals(true);
            ui->cb_wall_alignment->blockSignals(true);

            ui->sb_wall_length->setValue(wall->lengthInMeters());
            ui->sb_wall_thickness->setValue(wall->thicknessInMm());
            ui->sb_wall_angle->setValue(wall->angleInDegrees());
            ui->cb_wall_alignment->setCurrentIndex(wall->alignment());
            ui->lbl_wall_area->setText(QString("Чистая площадь: %1 м²").arg(wall->netArea(), 0, 'f', 2));

            ui->sb_wall_length->blockSignals(false);
            ui->sb_wall_thickness->blockSignals(false);
            ui->sb_wall_angle->blockSignals(false);
            ui->cb_wall_alignment->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (WindowItem *window = dynamic_cast<WindowItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(3);

        auto updateUI = [this, window]() {
            ui->sb_window_width->blockSignals(true);
            ui->sb_window_elevation->blockSignals(true);
            ui->cb_window_profile->blockSignals(true);
            ui->sb_window_distance->blockSignals(true);

            ui->sb_window_width->setValue(window->widthInMeters());
            ui->sb_window_elevation->setValue(window->elevation());
            ui->cb_window_profile->setCurrentIndex(window->profileType());
            ui->sb_window_distance->setValue(window->distanceFromStart());
            ui->lbl_window_area->setText(QString("Площадь проема: %1 м²").arg(window->area(), 0, 'f', 2));

            ui->sb_window_width->blockSignals(false);
            ui->sb_window_elevation->blockSignals(false);
            ui->cb_window_profile->blockSignals(false);
            ui->sb_window_distance->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (DoorItem *door = dynamic_cast<DoorItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(4);

        auto updateUI = [this, door]() {
            ui->sb_door_width->blockSignals(true);
            ui->sb_door_distance->blockSignals(true);
            ui->cb_door_swing->blockSignals(true);

            ui->sb_door_width->setValue(door->widthInMeters());
            ui->sb_door_distance->setValue(door->distanceFromStart());
            ui->cb_door_swing->setCurrentIndex(door->swingType());
            ui->lbl_door_area->setText(QString("Площадь проема: %1 м²").arg(door->area(), 0, 'f', 2));

            ui->sb_door_width->blockSignals(false);
            ui->sb_door_distance->blockSignals(false);
            ui->cb_door_swing->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
    else if (NodeItem *node = dynamic_cast<NodeItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(5);

        auto updateUI = [this, node]() {
            ui->sb_node_width->blockSignals(true);
            ui->sb_node_height->blockSignals(true);
            ui->sb_node_angle->blockSignals(true);
            ui->cb_node_type->blockSignals(true);

            ui->sb_node_width->setValue(node->side1InMeters());
            ui->sb_node_height->setValue(node->side2InMeters());
            ui->sb_node_angle->setValue(node->rotationAngle());
            ui->cb_node_type->setCurrentIndex(node->isRounded());
            ui->lbl_node_area->setText(QString("Площадь: %1 м²").arg(node->area(), 0, 'f', 4));

            ui->sb_node_width->blockSignals(false);
            ui->sb_node_height->blockSignals(false);
            ui->sb_node_angle->blockSignals(false);
            ui->cb_node_type->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
}

void EditorWindow::onFoundationPropertyChanged()
{
    if (m_trackedItem) {
        if (FoundationBlockItem *block = dynamic_cast<FoundationBlockItem*>(m_trackedItem)) {
            block->setWidthInMeters(ui->sb_found_width->value());
            block->setHeightInMeters(ui->sb_found_height->value());
            block->setRotation(ui->sb_found_angle->value());
        }
    }
}

void EditorWindow::onWallPropertyChanged()
{
    if (m_trackedItem) {
        if (WallItem *wall = dynamic_cast<WallItem*>(m_trackedItem)) {
            double length = ui->sb_wall_length->value();
            double thickness = ui->sb_wall_thickness->value();
            double angle = ui->sb_wall_angle->value();
            int alignment = ui->cb_wall_alignment->currentIndex();

            wall->setLengthInMeters(length);
            wall->setThicknessInMm(thickness);
            wall->setAngleInDegrees(angle);
            wall->setAlignment(alignment);
        }
    }
}

void EditorWindow::onWindowPropertyChanged()
{
    if (m_trackedItem) {
        if (WindowItem *window = dynamic_cast<WindowItem*>(m_trackedItem)) {
            double width = ui->sb_window_width->value();
            double elevation = ui->sb_window_elevation->value();
            int profile = ui->cb_window_profile->currentIndex();
            double distance = ui->sb_window_distance->value();

            window->setWidthInMeters(width);
            window->setElevation(elevation);
            window->setProfileType(profile);
            window->setDistanceFromStart(distance);
        }
    }
}

void EditorWindow::onDoorPropertyChanged()
{
    if (m_trackedItem) {
        if (DoorItem *door = dynamic_cast<DoorItem*>(m_trackedItem)) {
            double width = ui->sb_door_width->value();
            double distance = ui->sb_door_distance->value();
            int swingType = ui->cb_door_swing->currentIndex();

            door->setWidthInMeters(width);
            door->setDistanceFromStart(distance);
            door->setSwingType(swingType);
        }
    }
}

void EditorWindow::onNodePropertyChanged()
{
    if (m_trackedItem) {
        if (NodeItem *node = dynamic_cast<NodeItem*>(m_trackedItem)) {
            node->setSide1InMeters(ui->sb_node_width->value());
            node->setSide2InMeters(ui->sb_node_height->value());
            node->setRotationAngle(ui->sb_node_angle->value());
            node->setRounded(ui->cb_node_type->currentIndex());
        }
    }
}
