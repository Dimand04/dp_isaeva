#include "projectstagedialog.h"
#include "ui_projectstagedialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QVariant>
#include <QDate>

ProjectStageDialog::ProjectStageDialog(int projectId, int stageId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ProjectStageDialog)
    , m_projectId(projectId)
    , m_stageId(stageId)
{
    ui->setupUi(this);

    loadStatusesToComboBox();
    loadResponsibleUsers();

    connect(ui->chb_start_fact, &QCheckBox::toggled, this, &ProjectStageDialog::updateFactDateState);

    if (m_stageId == -1) {
        setWindowTitle("Добавление этапа");
        ui->pb_save->setText("Добавить");

        ui->de_start_plan->setDate(QDate::currentDate());
        ui->de_end_plan->setDate(QDate::currentDate().addDays(7));
        ui->de_start_fact->setDate(QDate::currentDate());

        ui->chb_start_fact->setChecked(false);
        int statusIndex = ui->cb_status->findData("pending");
        if (statusIndex >= 0) ui->cb_status->setCurrentIndex(statusIndex);

    } else {
        setWindowTitle("Редактирование этапа");
        ui->pb_save->setText("Сохранить");

        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            QSqlQuery query(db);
            query.prepare("SELECT stage_name, date_start_plan, date_end_plan, date_start_fact, status, responsible_id "
                          "FROM project_stages WHERE id = :id");
            query.bindValue(":id", m_stageId);

            if (query.exec() && query.next()) {

                ui->le_stage_name->setText(query.value("stage_name").toString());
                ui->de_start_plan->setDate(query.value("date_start_plan").toDate());
                ui->de_end_plan->setDate(query.value("date_end_plan").toDate());

                QVariant factDateVar = query.value("date_start_fact");
                if (factDateVar.isNull() || !factDateVar.toDate().isValid()) {
                    ui->chb_start_fact->setChecked(false);
                    ui->de_start_fact->setDate(QDate::currentDate());
                } else {
                    ui->chb_start_fact->setChecked(true);
                    ui->de_start_fact->setDate(factDateVar.toDate());
                }

                QString statusDb = query.value("status").toString();
                int statusIndex = ui->cb_status->findData(statusDb);
                if (statusIndex >= 0) ui->cb_status->setCurrentIndex(statusIndex);

                int respId = query.value("responsible_id").toInt();
                int respIndex = ui->cb_responsible->findData(respId);
                if (respIndex >= 0) ui->cb_responsible->setCurrentIndex(respIndex);

            }
        }
    }

    updateFactDateState();
}

ProjectStageDialog::~ProjectStageDialog()
{
    delete ui;
}

void ProjectStageDialog::on_pb_save_clicked()
{
    QString name = ui->le_stage_name->text().trimmed();
    QDate startPlan = ui->de_start_plan->date();
    QDate endPlan = ui->de_end_plan->date();

    QString status = ui->cb_status->currentData().toString();
    int responsibleId = ui->cb_responsible->currentData().toInt();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Название этапа не может быть пустым.");
        ui->le_stage_name->setFocus();
        return;
    }

    if (endPlan < startPlan) {
        QMessageBox::warning(this, "Внимание", "Плановая дата завершения не может быть раньше начала.");
        return;
    }

    QVariant startFactVar;
    if (ui->chb_start_fact->isChecked()) {
        startFactVar = ui->de_start_fact->date();
    } else {
        startFactVar = QVariant(QMetaType::fromType<QDate>());
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    if (m_stageId == -1) {
        query.prepare("INSERT INTO project_stages (project_id, stage_name, date_start_plan, date_end_plan, date_start_fact, status, responsible_id) "
                      "VALUES (:project_id, :stage_name, :date_start_plan, :date_end_plan, :date_start_fact, :status, :responsible_id)");
        query.bindValue(":project_id", m_projectId);
    } else {
        query.prepare("UPDATE project_stages SET stage_name = :stage_name, date_start_plan = :date_start_plan, "
                      "date_end_plan = :date_end_plan, date_start_fact = :date_start_fact, status = :status, "
                      "responsible_id = :responsible_id WHERE id = :id");
        query.bindValue(":id", m_stageId);
    }

    query.bindValue(":stage_name", name);
    query.bindValue(":date_start_plan", startPlan);
    query.bindValue(":date_end_plan", endPlan);
    query.bindValue(":date_start_fact", startFactVar);
    query.bindValue(":status", status);

    if (responsibleId == 0) {
        query.bindValue(":responsible_id", QVariant(QMetaType::fromType<int>()));
    } else {
        query.bindValue(":responsible_id", responsibleId);
    }

    if (query.exec()) {
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось сохранить этап:\n" + query.lastError().text());
    }
}

void ProjectStageDialog::updateFactDateState()
{
    ui->de_start_fact->setEnabled(ui->chb_start_fact->isChecked());
}

void ProjectStageDialog::loadStatusesToComboBox()
{
    ui->cb_status->clear();
    ui->cb_status->addItem("⏳ Ожидает", "pending");
    ui->cb_status->addItem("🔥 В работе", "in_progress");
    ui->cb_status->addItem("✅ Завершен", "done");
}

void ProjectStageDialog::loadResponsibleUsers()
{
    ui->cb_responsible->clear();
    ui->cb_responsible->addItem("Не назначен", 0);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    if (query.exec("SELECT id, login FROM users ORDER BY login ASC")) {
        while (query.next()) {
            int id = query.value("id").toInt();
            QString name = query.value("login").toString();
            ui->cb_responsible->addItem(name, id);
        }
    }
}
