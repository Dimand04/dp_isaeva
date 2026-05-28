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

    if (tableName == "categories") {
        ui->le_name->setPlaceholderText("Название категории");
    }

    if (m_itemId != -1) {
        ui->pb_save->setText("Обновить");

        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            QSqlQuery query(db);
            QString sql = QString("SELECT name FROM %1 WHERE id = :id").arg(m_tableName);
            query.prepare(sql);
            query.bindValue(":id", m_itemId);

            if (query.exec() && query.next()) {
                ui->le_name->setText(query.value(0).toString());
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

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    if (m_itemId == -1) {
        QString sql = QString("INSERT INTO %1 (name) VALUES (:name)").arg(m_tableName);
        query.prepare(sql);
        query.bindValue(":name", name);

        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось создать запись:\n" + query.lastError().text());
            return;
        }
    } else {
        QString sql = QString("UPDATE %1 SET name = :name WHERE id = :id").arg(m_tableName);
        query.prepare(sql);
        query.bindValue(":name", name);
        query.bindValue(":id", m_itemId);

        if (!query.exec()) {
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить запись:\n" + query.lastError().text());
            return;
        }
    }

    accept();
}

