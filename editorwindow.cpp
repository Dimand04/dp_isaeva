#include "editorwindow.h"
#include "ui_editorwindow.h"
#include "foundationblockitem.h"
#include "wallitem.h"

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
    connect(m_scene, &QGraphicsScene::selectionChanged, this, &EditorWindow::onSelectionChanged);
    connect(ui->sb_found_width, &QDoubleSpinBox::editingFinished, this, &EditorWindow::onFoundationPropertyChanged);
    connect(ui->sb_found_height, &QDoubleSpinBox::editingFinished, this, &EditorWindow::onFoundationPropertyChanged);
    connect(ui->sb_wall_length, &QDoubleSpinBox::editingFinished, this, &EditorWindow::onWallPropertyChanged);
    connect(ui->sb_wall_thickness, &QDoubleSpinBox::editingFinished, this, &EditorWindow::onWallPropertyChanged);
    connect(ui->sb_found_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EditorWindow::onFoundationPropertyChanged);

    connect(ui->sb_wall_angle, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &EditorWindow::onWallPropertyChanged);
}

EditorWindow::~EditorWindow()
{
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
        return;
    }

    QGraphicsItem *item = selected.first();
    m_trackedItem = dynamic_cast<BaseEditorItem*>(item);

    if (FoundationBlockItem *block = dynamic_cast<FoundationBlockItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(1);

        auto updateUI = [this, block]() {
            ui->sb_found_width->blockSignals(true);
            ui->sb_found_height->blockSignals(true);
            ui->sb_found_angle->blockSignals(true);

            ui->sb_found_width->setValue(block->widthInMeters());
            ui->sb_found_height->setValue(block->heightInMeters());
            ui->sb_found_angle->setValue(block->rotation());

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

            ui->sb_wall_length->setValue(wall->lengthInMeters());
            ui->sb_wall_thickness->setValue(wall->thicknessInMm());
            ui->sb_wall_angle->setValue(wall->angleInDegrees());

            ui->sb_wall_length->blockSignals(false);
            ui->sb_wall_thickness->blockSignals(false);
            ui->sb_wall_angle->blockSignals(false);
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
            wall->setLengthInMeters(ui->sb_wall_length->value());
            wall->setThicknessInMm(ui->sb_wall_thickness->value());
            wall->setAngleInDegrees(ui->sb_wall_angle->value());
        }
    }
}
