#include "projectestimatedialog.h"
#include "ui_projectestimatedialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

ProjectEstimateDialog::ProjectEstimateDialog(int projectId, int estimateId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProjectEstimateDialog)
    , m_projectId(projectId)
    , m_estimateId(estimateId)
{
    ui->setupUi(this);

    loadGroupTypes();
    setupConnections();

    if (m_estimateId == -1) {
        setWindowTitle("Добавление позиции в смету");
        ui->pb_save->setText("Добавить");

        ui->dsb_quantity->setValue(1.0);

        loadMaterials();

    } else {
        setWindowTitle("Редактирование позиции");
        ui->pb_save->setText("Сохранить");

        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            QSqlQuery query(db);
            query.prepare("SELECT group_type, material_id, quantity, price_per_unit "
                          "FROM project_estimates WHERE id = :id");
            query.bindValue(":id", m_estimateId);

            if (query.exec() && query.next()) {
                ui->cb_group_type->blockSignals(true);
                QString groupType = query.value("group_type").toString();
                int groupIndex = ui->cb_group_type->findData(groupType);
                if (groupIndex >= 0) ui->cb_group_type->setCurrentIndex(groupIndex);
                ui->cb_group_type->blockSignals(false);

                loadMaterials();

                ui->cb_material->blockSignals(true);
                int materialId = query.value("material_id").toInt();
                int materialIndex = ui->cb_material->findData(materialId);
                if (materialIndex >= 0) ui->cb_material->setCurrentIndex(materialIndex);
                ui->cb_material->blockSignals(false);

                ui->dsb_quantity->setValue(query.value("quantity").toDouble());
                ui->dsb_price->setValue(query.value("price_per_unit").toDouble());

                calculateTotal();
            }
        }
    }
}

ProjectEstimateDialog::~ProjectEstimateDialog()
{
    delete ui;
}

void ProjectEstimateDialog::onGroupTypeChanged(int index)
{
    loadMaterials();
    ui->dsb_price->setValue(0.0);
    calculateTotal();
}

void ProjectEstimateDialog::onMaterialChanged(int index)
{
    fetchMaterialPrice();
}

void ProjectEstimateDialog::calculateTotal()
{
    double quantity = ui->dsb_quantity->value();
    double price = ui->dsb_price->value();
    double total = quantity * price;

    ui->dsb_total_sum->setValue(total);
}

void ProjectEstimateDialog::on_pb_save_clicked()
{
    QString groupType = ui->cb_group_type->currentData().toString();
    int materialId = ui->cb_material->currentData().toInt();
    double quantity = ui->dsb_quantity->value();
    double price = ui->dsb_price->value();

    if (materialId == 0) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, выберите позицию из справочника.");
        return;
    }

    if (quantity <= 0.0) {
        QMessageBox::warning(this, "Внимание", "Количество должно быть больше нуля.");
        ui->dsb_quantity->setFocus();
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    if (m_estimateId == -1) {
        query.prepare("INSERT INTO project_estimates (project_id, material_id, quantity, price_per_unit, group_type) "
                      "VALUES (:project_id, :material_id, :quantity, :price_per_unit, :group_type)");
        query.bindValue(":project_id", m_projectId);
    } else {
        query.prepare("UPDATE project_estimates SET material_id = :material_id, quantity = :quantity, "
                      "price_per_unit = :price_per_unit, group_type = :group_type "
                      "WHERE id = :id");
        query.bindValue(":id", m_estimateId);
    }

    query.bindValue(":material_id", materialId);
    query.bindValue(":quantity", quantity);
    query.bindValue(":price_per_unit", price);
    query.bindValue(":group_type", groupType);

    if (query.exec()) {
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось сохранить позицию сметы:\n" + query.lastError().text());
    }
}

void ProjectEstimateDialog::setupConnections()
{
    connect(ui->cb_group_type, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProjectEstimateDialog::onGroupTypeChanged);

    connect(ui->cb_material, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProjectEstimateDialog::onMaterialChanged);

    connect(ui->dsb_quantity, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ProjectEstimateDialog::calculateTotal);

    connect(ui->dsb_price, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &ProjectEstimateDialog::calculateTotal);
}

void ProjectEstimateDialog::loadGroupTypes()
{
    ui->cb_group_type->blockSignals(true);
    ui->cb_group_type->clear();
    ui->cb_group_type->addItem("📦 Материалы", "material");
    ui->cb_group_type->addItem("👷 Работы", "work");
    ui->cb_group_type->addItem("🚚 Услуги", "service");
    ui->cb_group_type->blockSignals(false);
}

void ProjectEstimateDialog::loadMaterials()
{
    ui->cb_material->blockSignals(true);
    ui->cb_material->clear();
    ui->cb_material->addItem("Выберите позицию", 0);

    QString currentGroup = ui->cb_group_type->currentData().toString();

    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT m.id, m.name "
                      "FROM materials m "
                      "JOIN categories c ON m.category_id = c.id "
                      "WHERE c.type = :type "
                      "ORDER BY m.name ASC");
        query.bindValue(":type", currentGroup);

        if (query.exec()) {
            while (query.next()) {
                int id = query.value("id").toInt();
                QString name = query.value("name").toString();
                ui->cb_material->addItem(name, id);
            }
        } else {
            qWarning() << "Ошибка загрузки номенклатуры:" << query.lastError().text();
        }
    }
    ui->cb_material->blockSignals(false);
}

void ProjectEstimateDialog::fetchMaterialPrice()
{
    int materialId = ui->cb_material->currentData().toInt();
    if (materialId == 0) {
        ui->dsb_price->setValue(0.0);
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT base_price FROM materials WHERE id = :id");
        query.bindValue(":id", materialId);

        if (query.exec() && query.next()) {
            ui->dsb_price->blockSignals(true);
            ui->dsb_price->setValue(query.value("base_price").toDouble());
            ui->dsb_price->blockSignals(false);

            calculateTotal();
        }
    }
}
