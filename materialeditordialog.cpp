#include "materialeditordialog.h"
#include "ui_materialeditordialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

MaterialEditorDialog::MaterialEditorDialog(int materialId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MaterialEditorDialog)
    , m_materialId(materialId)
{
    ui->setupUi(this);

    loadDictionaries();

    if (m_materialId == -1) {
        setWindowTitle("Создание материала");
        ui->pb_save->setText("Создать");

        ui->le_name->clear();
        ui->dsb_base_cost->setValue(0.00);

    } else {
        setWindowTitle("Редактирование материала");
        ui->pb_save->setText("Обновить");

        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            QSqlQuery query(db);

            query.prepare("SELECT name, category_id, unit_id, base_price FROM materials WHERE id = :id");
            query.bindValue(":id", m_materialId);

            if (query.exec() && query.next()) {

                ui->le_name->setText(query.value(0).toString());

                ui->dsb_base_cost->setValue(query.value(3).toDouble());

                int categoryId = query.value(1).toInt();
                int catIndex = ui->cb_categories->findData(categoryId);
                if (catIndex != -1) {
                    ui->cb_categories->setCurrentIndex(catIndex);
                }

                int unitId = query.value(2).toInt();
                int unitIndex = ui->cb_units->findData(unitId);
                if (unitIndex != -1) {
                    ui->cb_units->setCurrentIndex(unitIndex);
                }

            } else {
                qWarning() << "Ошибка при загрузке данных материала:" << query.lastError().text();
            }
        }
    }
}

MaterialEditorDialog::~MaterialEditorDialog()
{
    delete ui;
}

void MaterialEditorDialog::loadComboBox(QComboBox *cb, const QString &tableName, const QString &defaultText)
{
    cb->clear();
    cb->addItem(defaultText, -1);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare(QString("SELECT id, name FROM %1 ORDER BY name ASC").arg(tableName));

    if (query.exec()) {
        while (query.next()) {
            int id = query.value(0).toInt();
            QString name = query.value(1).toString();

            cb->addItem(name, id);
        }
    } else {
        qWarning() << "Ошибка загрузки справочника" << tableName << ":" << query.lastError().text();
    }
}

void MaterialEditorDialog::loadDictionaries()
{
    loadComboBox(ui->cb_categories, "categories", "Выберите категорию");

    loadComboBox(ui->cb_units, "units", "Выберите ед. измерения");
}

void MaterialEditorDialog::on_pb_save_clicked()
{
    QString name = ui->le_name->text().trimmed();
    int categoryId = ui->cb_categories->currentData().toInt();
    int unitId = ui->cb_units->currentData().toInt();
    double base_price = ui->dsb_base_cost->value();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, введите название материала.");
        ui->le_name->setFocus();
        return;
    }

    if (categoryId == -1) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, выберите категорию материала.");
        return;
    }

    if (unitId == -1) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, выберите единицу измерения.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Ошибка", "Нет подключения к базе данных.");
        return;
    }

    QSqlQuery query(db);

    if (m_materialId == -1) {
        query.prepare("INSERT INTO materials (name, category_id, unit_id, base_price) "
                      "VALUES (:name, :category_id, :unit_id, :base_price)");
    } else {
        query.prepare("UPDATE materials SET name = :name, category_id = :category_id, "
                      "unit_id = :unit_id, base_price = :base_price WHERE id = :id");

        query.bindValue(":id", m_materialId);
    }

    query.bindValue(":name", name);
    query.bindValue(":category_id", categoryId);
    query.bindValue(":unit_id", unitId);
    query.bindValue(":base_price", base_price);

    if (query.exec()) {
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось сохранить материал:\n" + query.lastError().text());
    }
}
