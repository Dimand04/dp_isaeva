#include "clienteditordialog.h"
#include "ui_clienteditordialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

ClientEditorDialog::ClientEditorDialog(int clientId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClientEditorDialog)
    , m_clientId(clientId)
{
    ui->setupUi(this);

    connect(ui->rb_individual, &QRadioButton::toggled, this, &ClientEditorDialog::updatePlaceholders);
    connect(ui->rb_legal, &QRadioButton::toggled, this, &ClientEditorDialog::updatePlaceholders);

    if (m_clientId == -1) {
        setWindowTitle("Создание клиента");
        ui->pb_save->setText("Создать");
        ui->rb_individual->setChecked(true);

    } else {
        setWindowTitle("Редактирование клиента");
        ui->pb_save->setText("Обновить");
        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            QSqlQuery query(db);
            query.prepare("SELECT type, name, phone, email, passport, address, notes FROM clients WHERE id = :id");
            query.bindValue(":id", m_clientId);

            if (query.exec() && query.next()) {

                QString type = query.value("type").toString();
                if (type == "legal") {
                    ui->rb_legal->setChecked(true);
                } else {
                    ui->rb_individual->setChecked(true);
                }

                ui->le_name->setText(query.value("name").toString());
                ui->le_phone->setText(query.value("phone").toString());
                ui->le_email->setText(query.value("email").toString());
                ui->le_passport->setText(query.value("passport").toString());

                ui->pte_address->setPlainText(query.value("address").toString());
                ui->pte_notes->setPlainText(query.value("notes").toString());

            } else {
                qWarning() << "Ошибка при загрузке данных клиента для редактирования:" << query.lastError().text();
            }
        }
    }

    updatePlaceholders();
}

ClientEditorDialog::~ClientEditorDialog()
{
    delete ui;
}

void ClientEditorDialog::updatePlaceholders()
{
    if (ui->rb_individual->isChecked()) {
        ui->le_name->setPlaceholderText("ФИО");
        ui->le_passport->setPlaceholderText("Паспортные данные");
    } else {
        ui->le_name->setPlaceholderText("Название организации");
        ui->le_passport->setPlaceholderText("ИНН / ОГРН");
    }
}

void ClientEditorDialog::on_pb_save_clicked()
{
    QString type = ui->rb_legal->isChecked() ? "legal" : "individual";

    QString name = ui->le_name->text().trimmed();

    QString phone = ui->le_phone->text().trimmed();

    QString email = ui->le_email->text().trimmed();
    QString passport = ui->le_passport->text().trimmed();

    QString address = ui->pte_address->toPlainText().trimmed();
    QString notes = ui->pte_notes->toPlainText().trimmed();

    if (name.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, введите ФИО или название организации.");
        ui->le_name->setFocus();
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        QMessageBox::critical(this, "Ошибка", "Нет подключения к базе данных.");
        return;
    }

    QSqlQuery query(db);

    if (m_clientId == -1) {
        query.prepare("INSERT INTO clients (type, name, phone, email, passport, address, notes, created_at) "
                      "VALUES (:type, :name, :phone, :email, :passport, :address, :notes, CURRENT_TIMESTAMP)");
    } else {
        query.prepare("UPDATE clients SET type = :type, name = :name, phone = :phone, "
                      "email = :email, passport = :passport, address = :address, notes = :notes "
                      "WHERE id = :id");

        query.bindValue(":id", m_clientId);
    }

    query.bindValue(":type", type);
    query.bindValue(":name", name);
    query.bindValue(":phone", phone);
    query.bindValue(":email", email);
    query.bindValue(":passport", passport);
    query.bindValue(":address", address);
    query.bindValue(":notes", notes);

    if (query.exec()) {
        accept();
    } else {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось сохранить данные клиента:\n" + query.lastError().text());
    }
}
