#include "simpledicteditordialog.h"
#include "ui_simpledicteditordialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

SimpleDictEditorDialog::SimpleDictEditorDialog(const QString &tableName, const QString &windowTitle, int itemId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SimpleDictEditorDialog)
    , m_tableName(tableName)
    , m_itemId(itemId)
{
    ui->setupUi(this);

    setWindowTitle(windowTitle);

    bool isCategories = (m_tableName == "categories");

    ui->cb_type->setVisible(isCategories);
    ui->lbl_type->setVisible(isCategories);

    if (isCategories) {
        ui->le_name->setPlaceholderText("Название категории");
        ui->cb_type->clear();
        ui->cb_type->addItem("Материал", "material");
        ui->cb_type->addItem("Работа", "work");
        ui->cb_type->addItem("Услуга", "service");
    }

    if (m_itemId != -1) {
        ui->pb_save->setText("Обновить");

        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            QSqlQuery query(db);
            QString sql;

            if (isCategories) {
                sql = QString("SELECT name, type FROM %1 WHERE id = :id").arg(m_tableName);
            } else {
                sql = QString("SELECT name FROM %1 WHERE id = :id").arg(m_tableName);
            }

            query.prepare(sql);
            query.bindValue(":id", m_itemId);

            if (query.exec() && query.next()) {
                ui->le_name->setText(query.value("name").toString());

                if (isCategories) {
                    QString typeDb = query.value("type").toString();
                    int idx = ui->cb_type->findData(typeDb);
                    if (idx >= 0) {
                        ui->cb_type->setCurrentIndex(idx);
                    }
                }
            } else {
                qWarning() << "Ошибка при загрузке данных справочника:" << query.lastError().text();
            }
        }
    } else {
        ui->pb_save->setText("Создать");
    }
}

SimpleDictEditorDialog::~SimpleDictEditorDialog()
{
    delete ui;
}

void SimpleDictEditorDialog::on_pb_save_clicked()
{
    QString name = ui->le_name->text().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Название не может быть пустым.");
        return;
    }

    bool isCategories = (m_tableName == "categories");
    QString type;

    if (isCategories) {
        type = ui->cb_type->currentData().toString();
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    if (m_itemId == -1) {
        QString sql;
        if (isCategories) {
            sql = QString("INSERT INTO %1 (name, type) VALUES (:name, :type)").arg(m_tableName);
        } else {
            sql = QString("INSERT INTO %1 (name) VALUES (:name)").arg(m_tableName);
        }

        query.prepare(sql);
        query.bindValue(":name", name);
        if (isCategories) {
            query.bindValue(":type", type);
        }

        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось создать запись:\n" + query.lastError().text());
            return;
        }
    } else {
        QString sql;
        if (isCategories) {
            sql = QString("UPDATE %1 SET name = :name, type = :type WHERE id = :id").arg(m_tableName);
        } else {
            sql = QString("UPDATE %1 SET name = :name WHERE id = :id").arg(m_tableName);
        }

        query.prepare(sql);
        query.bindValue(":name", name);
        if (isCategories) {
            query.bindValue(":type", type);
        }
        query.bindValue(":id", m_itemId);

        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить запись:\n" + query.lastError().text());
            return;
        }
    }

    accept();
}
