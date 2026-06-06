#include "projecteditordialog.h"
#include "ui_projecteditordialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDate>

ProjectEditorDialog::ProjectEditorDialog(int projectId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProjectEditorDialog)
    , m_projectId(projectId)
{
    ui->setupUi(this);

    loadStatusesToComboBox();
    loadClientsToComboBox();

    if (m_projectId == -1) {
        setWindowTitle("Создание нового проекта");

        ui->pb_save->setText("Создать");

        ui->de_start_date->setDate(QDate::currentDate());
        ui->de_end_date->setDate(QDate::currentDate().addMonths(1));

    } else {
        setWindowTitle("Редактирование проекта");

        ui->pb_save->setText("Обновить");

        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            QSqlQuery query(db);
            query.prepare("SELECT client_id, name, address, status, start_date, end_date, total_cost "
                          "FROM projects WHERE id = :id");
            query.bindValue(":id", m_projectId);

            if (query.exec() && query.next()) {

                ui->le_name->setText(query.value("name").toString());
                ui->pte_address->setPlainText(query.value("address").toString());
                ui->dsb_total_cost->setValue(query.value("total_cost").toDouble());

                ui->de_start_date->setDate(query.value("start_date").toDate());
                ui->de_end_date->setDate(query.value("end_date").toDate());

                int clientId = query.value("client_id").toInt();
                int clientIndex = ui->cb_clients->findData(clientId);
                if (clientIndex >= 0) ui->cb_clients->setCurrentIndex(clientIndex);

                QString statusDb = query.value("status").toString();
                int statusIndex = ui->cb_statuses->findData(statusDb);
                if (statusIndex >= 0) ui->cb_statuses->setCurrentIndex(statusIndex);

            } else {
                qWarning() << "Ошибка при загрузке данных проекта:" << query.lastError().text();
            }
        }
    }
}

ProjectEditorDialog::~ProjectEditorDialog()
{
    delete ui;
}

void ProjectEditorDialog::loadStatusesToComboBox()
{
    ui->cb_statuses->clear();
    ui->cb_statuses->addItem("✏️ Проектирование", "design");
    ui->cb_statuses->addItem("🏗️ Строительство", "building");
    ui->cb_statuses->addItem("✅ Завершено", "finished");
    ui->cb_statuses->addItem("❄️ Заморожено", "frozen");
}

void ProjectEditorDialog::loadClientsToComboBox()
{
    ui->cb_clients->clear();

    ui->cb_clients->addItem("Выберите клиента", 0);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    if (query.exec("SELECT id, name FROM clients ORDER BY name ASC")) {
        while (query.next()) {
            int id = query.value("id").toInt();
            QString name = query.value("name").toString();

            ui->cb_clients->addItem(name, id);
        }
    }
}

void ProjectEditorDialog::on_pb_save_clicked()
{
    QString name = ui->le_name->text().trimmed();
    QString address = ui->pte_address->toPlainText().trimmed();

    int clientId = ui->cb_clients->currentData().toInt();
    QString status = ui->cb_statuses->currentData().toString();

    QDate startDate = ui->de_start_date->date();
    QDate endDate = ui->de_end_date->date();
    double totalCost = ui->dsb_total_cost->value();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, введите название проекта.");
        ui->le_name->setFocus();
        return;
    }
    if (clientId == 0) {
        QMessageBox::warning(this, "Внимание", "Для проекта должен быть выбран заказчик.\n\nСначала создайте хотя бы одного клиента в базе.");
        return;
    }
    if (endDate < startDate) {
        QMessageBox::warning(this, "Внимание", "Дата завершения не может быть раньше даты начала.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    if (m_projectId == -1) {
        query.prepare("INSERT INTO projects (client_id, name, address, status, start_date, end_date, total_cost, schema_data) "
                      "VALUES (:client_id, :name, :address, :status, :start_date, :end_date, :total_cost, '{}')");
    } else {
        query.prepare("UPDATE projects SET client_id = :client_id, name = :name, address = :address, "
                      "status = :status, start_date = :start_date, end_date = :end_date, total_cost = :total_cost "
                      "WHERE id = :id");
        query.bindValue(":id", m_projectId);
    }

    query.bindValue(":client_id", clientId);
    query.bindValue(":name", name);
    query.bindValue(":address", address);
    query.bindValue(":status", status);
    query.bindValue(":start_date", startDate);
    query.bindValue(":end_date", endDate);
    query.bindValue(":total_cost", totalCost);

    if (query.exec()) {
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось сохранить проект:\n" + query.lastError().text());
    }
}

void ProjectEditorDialog::setClient(int clientId)
{
    int index = ui->cb_clients->findData(clientId);
    if (index >= 0) {
        ui->cb_clients->setCurrentIndex(index);
    }
}

