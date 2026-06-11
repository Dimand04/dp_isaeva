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
    // Получаем список всех выделенных объектов
    QList<QGraphicsItem*> selected = m_scene->selectedItems();

    // Если ничего не выделено или выделено больше одного объекта - показываем пустую панель
    if (selected.isEmpty() || selected.size() > 1) {
        ui->stackedWidget->setCurrentIndex(0);
        return;
    }

    QGraphicsItem *item = selected.first();

    // Пытаемся преобразовать базовый элемент в конкретные классы
    if (FoundationBlockItem *block = dynamic_cast<FoundationBlockItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(1); // Страница фундамента

        // ВАЖНО: блокируем сигналы перед изменением значений, чтобы не вызвать
        // бесконечный цикл обновлений (когда UI меняет объект, а объект снова меняет UI)
        ui->sb_found_width->blockSignals(true);
        ui->sb_found_height->blockSignals(true);

        ui->sb_found_width->setValue(block->widthInMeters());
        ui->sb_found_height->setValue(block->heightInMeters());

        ui->sb_found_width->blockSignals(false);
        ui->sb_found_height->blockSignals(false);
    }
    else if (WallItem *wall = dynamic_cast<WallItem*>(item)) {
        ui->stackedWidget->setCurrentIndex(2); // Страница стены

        ui->sb_wall_length->blockSignals(true);
        ui->sb_wall_thickness->blockSignals(true);

        ui->sb_wall_length->setValue(wall->lengthInMeters());
        ui->sb_wall_thickness->setValue(wall->thicknessInMm());

        ui->sb_wall_length->blockSignals(false);
        ui->sb_wall_thickness->blockSignals(false);
    }
}

void EditorWindow::onFoundationPropertyChanged()
{
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    if (selected.size() == 1) {
        if (FoundationBlockItem *block = dynamic_cast<FoundationBlockItem*>(selected.first())) {
            block->setWidthInMeters(ui->sb_found_width->value());
            block->setHeightInMeters(ui->sb_found_height->value());
        }
    }
}

void EditorWindow::onWallPropertyChanged()
{
    QList<QGraphicsItem*> selected = m_scene->selectedItems();
    if (selected.size() == 1) {
        if (WallItem *wall = dynamic_cast<WallItem*>(selected.first())) {
            wall->setLengthInMeters(ui->sb_wall_length->value());
            wall->setThicknessInMm(ui->sb_wall_thickness->value());
        }
    }
}
