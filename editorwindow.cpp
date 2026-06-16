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

    connect(ui->pb_tool_cursor, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_foundation, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_wall, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_node, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_window, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_door, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_floor, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);
    connect(ui->pb_tool_dimension, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);

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

    connect(ui->cb_dim_side, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &EditorWindow::onDimensionPropertyChanged);

    m_coordLabel = new QLabel("Клетка X: 0 | Y: 0", this);
    ui->statusBar->addPermanentWidget(m_coordLabel);

    connect(m_scene, &EditorScene::cursorMoved, this, [this](const QPointF &pos) {
        if (m_coordLabel) {
            m_coordLabel->setText(QString("X: %1   Y: %2")
                                      .arg(qRound(pos.x()))
                                      .arg(qRound(pos.y())));
        }
    });

    connect(ui->pb_tool_text, &QPushButton::clicked, this, &EditorWindow::onToolButtonClicked);

    connect(ui->le_text_content, &QLineEdit::textEdited, this, &EditorWindow::onTextPropertyChanged);

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
    });

    connect(ui->sb_node_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onNodePropertyChanged);
    connect(ui->sb_node_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onNodePropertyChanged);
    connect(ui->sb_node_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onNodePropertyChanged);
    connect(ui->cb_node_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onNodePropertyChanged);

    connect(ui->le_object_foundation_name, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_trackedItem) m_trackedItem->setName(text);
    });

    connect(ui->le_object_wall_name, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_trackedItem) m_trackedItem->setName(text);
    });

    connect(ui->le_object_window_name, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_trackedItem) m_trackedItem->setName(text);
    });

    connect(ui->le_object_door_name, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_trackedItem) m_trackedItem->setName(text);
    });

    connect(ui->le_object_node_name, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_trackedItem) m_trackedItem->setName(text);
    });

    connect(ui->le_object_floor_name, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_trackedItem) m_trackedItem->setName(text);
    });

    connect(ui->le_object_line_name, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_trackedItem) m_trackedItem->setName(text);
    });

    connect(ui->le_object_text_name, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_trackedItem) m_trackedItem->setName(text);
    });

    connect(ui->sb_workspace_width, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
        m_scene->setWorkspaceSize(val, ui->sb_workspace_height->value());
    });

    connect(ui->sb_workspace_height, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double val) {
        m_scene->setWorkspaceSize(ui->sb_workspace_width->value(), val);
    });

    connect(ui->sb_text_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onTextPropertyChanged);

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
        int levelId = text.toInt();

        if (m_scene) m_scene->setActiveLevel(levelId);
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

            if (m_scene) {
                m_scene->setLayerVisible(layerName, isVisible);
            }
        }
    });

    connect(ui->le_object_roof_name, &QLineEdit::textEdited, this, [this](const QString &text) {
        if (m_trackedItem) m_trackedItem->setName(text);
    });

    connect(ui->cb_roof_type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditorWindow::onRoofPropertyChanged);
    connect(ui->sb_roof_overhang, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onRoofPropertyChanged);
    connect(ui->sb_roof_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &EditorWindow::onRoofPropertyChanged);
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
    } else if (button == ui->pb_tool_dimension) {
        m_scene->setToolMode(ModeDimension);
    } else if (button == ui->pb_tool_text) {
        m_scene->setToolMode(ModeText);
    } else if (button == ui->pb_tool_roof) {
        m_scene->setToolMode(ModeRoof);
    }
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
            ui->sb_found_angle->blockSignals(true);

            ui->le_object_foundation_name->setText(block->name());
            ui->sb_found_width->setValue(block->widthInMeters());
            ui->sb_found_height->setValue(block->heightInMeters());
            ui->sb_found_angle->setValue(block->rotation());
            ui->lbl_found_area->setText(QString("Площадь: %1 м²").arg(block->area(), 0, 'f', 2));

            ui->le_object_foundation_name->blockSignals(false);
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
            ui->le_object_wall_name->blockSignals(true);
            ui->sb_wall_length->blockSignals(true);
            ui->sb_wall_thickness->blockSignals(true);
            ui->sb_wall_angle->blockSignals(true);
            ui->cb_wall_alignment->blockSignals(true);

            ui->le_object_wall_name->setText(wall->name());
            ui->sb_wall_length->setValue(wall->lengthInMeters());
            ui->sb_wall_thickness->setValue(wall->thicknessInMm());
            ui->sb_wall_angle->setValue(wall->angleInDegrees());
            ui->cb_wall_alignment->setCurrentIndex(wall->alignment());
            ui->lbl_wall_area->setText(QString("Чистая площадь: %1 м²").arg(wall->netArea(), 0, 'f', 2));

            ui->le_object_wall_name->blockSignals(false);
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
            ui->le_object_window_name->blockSignals(true);
            ui->sb_window_width->blockSignals(true);
            ui->sb_window_elevation->blockSignals(true);
            ui->cb_window_profile->blockSignals(true);
            ui->sb_window_distance->blockSignals(true);

            ui->le_object_window_name->setText(window->name());
            ui->sb_window_width->setValue(window->widthInMeters());
            ui->sb_window_elevation->setValue(window->elevation());
            ui->cb_window_profile->setCurrentIndex(window->profileType());
            ui->sb_window_distance->setValue(window->distanceFromStart());
            ui->lbl_window_area->setText(QString("Площадь проема: %1 м²").arg(window->area(), 0, 'f', 2));

            ui->le_object_window_name->blockSignals(false);
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
            ui->le_object_door_name->blockSignals(true);
            ui->sb_door_width->blockSignals(true);
            ui->sb_door_distance->blockSignals(true);
            ui->cb_door_swing->blockSignals(true);

            ui->le_object_door_name->setText(door->name());
            ui->sb_door_width->setValue(door->widthInMeters());
            ui->sb_door_distance->setValue(door->distanceFromStart());
            ui->cb_door_swing->setCurrentIndex(door->swingType());
            ui->lbl_door_area->setText(QString("Площадь проема: %1 м²").arg(door->area(), 0, 'f', 2));

            ui->le_object_door_name->blockSignals(false);
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
            ui->le_object_floor_name->setText(floor->name());
            ui->lbl_floor_area->setText(QString("Площадь пола: %1 м²").arg(floor->area(), 0, 'f', 2));
            ui->le_object_floor_name->blockSignals(false);
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
    } else if (RoofItem *roof = dynamic_cast<RoofItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(9);

        auto updateUI = [this, roof]() {
            ui->le_object_roof_name->blockSignals(true);
            ui->cb_roof_type->blockSignals(true);
            ui->sb_roof_overhang->blockSignals(true);
            ui->sb_roof_angle->blockSignals(true);

            ui->le_object_roof_name->setText(roof->name());
            ui->cb_roof_type->setCurrentIndex(roof->roofType());
            ui->sb_roof_overhang->setValue(roof->overhang());
            ui->sb_roof_angle->setValue(roof->angle());

            ui->lbl_roof_area->setText(QString("Площадь: %1 м²").arg(roof->area(), 0, 'f', 2));

            ui->le_object_roof_name->blockSignals(false);
            ui->cb_roof_type->blockSignals(false);
            ui->sb_roof_overhang->blockSignals(false);
            ui->sb_roof_angle->blockSignals(false);
        };

        updateUI();
        connect(m_trackedItem, &BaseEditorItem::itemChanged, this, updateUI);
    }
}

void EditorWindow::onFoundationPropertyChanged()
{
    if (m_trackedItem) {
        if (FoundationBlockItem *block = dynamic_cast<FoundationBlockItem*>(m_trackedItem)) {
            QString newName = ui->le_object_foundation_name->text();
            if (block->name() != newName) block->setName(newName);

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
            QString newName = ui->le_object_wall_name->text();
            if (wall->name() != newName) wall->setName(newName);

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
            QString newName = ui->le_object_window_name->text();
            if (window->name() != newName) window->setName(newName);

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
            QString newName = ui->le_object_door_name->text();
            if (door->name() != newName) door->setName(newName);

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

            int side = ui->cb_dim_side->currentIndex();
            dim->setTextSide(side);
        }
    }
}

void EditorWindow::onFloorPropertyChanged()
{
    if (m_trackedItem) {
        if (FloorItem *floor = dynamic_cast<FloorItem*>(m_trackedItem)) {
            QString newName = ui->le_object_floor_name->text();
            if (floor->name() != newName) floor->setName(newName);
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

void EditorWindow::onRoofPropertyChanged()
{
    if (m_trackedItem) {
        if (RoofItem *roof = dynamic_cast<RoofItem*>(m_trackedItem)) {
            QString newName = ui->le_object_roof_name->text();
            if (roof->name() != newName) roof->setName(newName);

            roof->setRoofType(ui->cb_roof_type->currentIndex());
            roof->setOverhang(ui->sb_roof_overhang->value());
            roof->setAngle(ui->sb_roof_angle->value());
        }
    }
}
