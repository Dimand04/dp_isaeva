#include "paymentdialog.h"
#include "ui_paymentdialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDateTime>

PaymentDialog::PaymentDialog(int clientId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PaymentDialog)
    , m_clientId(clientId)
{
    ui->setupUi(this);

    setWindowTitle("Внесение платежа");

    ui->dte_date->setDateTime(QDateTime::currentDateTime());

    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT id, name FROM projects WHERE client_id = :client_id AND status != 'finished'");
        query.bindValue(":client_id", m_clientId);

        if (query.exec()) {
            while (query.next()) {
                int projectId = query.value("id").toInt();
                QString projectName = query.value("name").toString();

                ui->cb_project->addItem(projectName, projectId);
            }
        }
    }

    if (ui->cb_project->count() == 0) {
        ui->cb_project->addItem("Нет доступных проектов");
        ui->cb_project->setEnabled(false);
        ui->pb_save->setEnabled(false);
    }
}

PaymentDialog::~PaymentDialog()
{
    delete ui;
}

void PaymentDialog::on_pb_save_clicked()
{
    double amount = ui->dsb_amount->value();
    if (amount <= 0.0) {
        QMessageBox::warning(this, "Ошибка", "Сумма платежа должна быть больше нуля!");
        return;
    }

    int projectId = ui->cb_project->currentData().toInt();
    QDateTime paymentDate = ui->dte_date->dateTime();
    QString purpose = ui->le_purpose->text().trimmed();

    if (purpose.isEmpty()) {
        purpose = "Оплата по договору";
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    query.prepare("INSERT INTO client_payments (project_id, amount, payment_date, purpose) "
                  "VALUES (:project_id, :amount, :payment_date, :purpose)");

    query.bindValue(":project_id", projectId);
    query.bindValue(":amount", amount);
    query.bindValue(":payment_date", paymentDate);
    query.bindValue(":purpose", purpose);

    if (query.exec()) {
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось провести платеж:\n" + query.lastError().text());
    }
}
