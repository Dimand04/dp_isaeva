#include "usereditordialog.h"
#include "ui_usereditordialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

UserEditorDialog::UserEditorDialog(int userId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UserEditorDialog)
    , m_userId(userId)
{
    ui->setupUi(this);

    loadRoles();

    if (m_userId == -1) {
        setWindowTitle("Создание нового пользователя");
        ui->pb_ok->setText("Создать");
        ui->le_password->setPlaceholderText("Пароль");
        ui->le_login->clear();
    } else {
        setWindowTitle("Редактирование пользователя");
        ui->pb_ok->setText("Обновить");
        ui->le_password->setPlaceholderText("Новый пароль");

        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            QSqlQuery query(db);
            query.prepare("SELECT login, role_id FROM users WHERE id = :id");
            query.bindValue(":id", m_userId);

            if (query.exec() && query.next()) {
                ui->le_login->setText(query.value(0).toString());

                int roleId = query.value(1).toInt();

                int index = ui->cb_roles->findData(roleId);

                if (index != -1) {
                    ui->cb_roles->setCurrentIndex(index);
                }

            } else {
                qWarning() << "Ошибка при загрузке данных пользователя:" << query.lastError().text();
            }
        }
    }
}

UserEditorDialog::~UserEditorDialog()
{
    delete ui;
}

void UserEditorDialog::loadRoles()
{
    ui->cb_roles->clear();

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "База данных не открыта!";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id, name FROM roles ORDER BY name ASC");

    if (!query.exec()) {
        qCritical() << "Ошибка при загрузке списка ролей в комбобокс:" << query.lastError().text();
        return;
    }

    ui->cb_roles->addItem("Выберите роль", -1);

    while (query.next()) {
        int roleId = query.value(0).toInt();
        QString roleName = query.value(1).toString();

        ui->cb_roles->addItem(roleName, roleId);
    }
}

void UserEditorDialog::on_pb_ok_clicked()
{
    QString login = ui->le_login->text().trimmed();
    QString password = ui->le_password->text();
    int roleId = ui->cb_roles->currentData().toInt();

    if (login.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Поле логина не может быть пустым.");
        return;
    }

    if (roleId == -1) {
        QMessageBox::warning(this, "Ошибка", "Пожалуйста, назначьте роль пользователю.");
        return;
    }

    if (m_userId == -1 && password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Для нового пользователя необходимо задать пароль.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось начать транзакцию базы данных.");
        return;
    }

    QSqlQuery query(db);

    if (m_userId == -1) {

        query.prepare("INSERT INTO users (login, role_id) VALUES (:login, :role_id)");
        query.bindValue(":login", login);
        query.bindValue(":role_id", roleId);

        if (!query.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Ошибка", "Не удалось создать пользователя:\n" + query.lastError().text());
            return;
        }

        int newUserId = query.lastInsertId().toInt();

        query.prepare("INSERT INTO auth (id, user_password) VALUES (:id, :pass)");
        query.bindValue(":id", newUserId);
        query.bindValue(":pass", password);

        if (!query.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Ошибка", "Не удалось сохранить пароль:\n" + query.lastError().text());
            return;
        }

    } else {

        query.prepare("UPDATE users SET login = :login, role_id = :role_id WHERE id = :id");
        query.bindValue(":login", login);
        query.bindValue(":role_id", roleId);
        query.bindValue(":id", m_userId);

        if (!query.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить данные пользователя:\n" + query.lastError().text());
            return;
        }

        if (!password.isEmpty()) {

            query.prepare("UPDATE auth SET user_password = :pass, remember_token = NULL WHERE id = :id");
            query.bindValue(":pass", password);
            query.bindValue(":id", m_userId);

            if (!query.exec()) {
                db.rollback();
                QMessageBox::critical(this, "Ошибка", "Не удалось обновить пароль:\n" + query.lastError().text());
                return;
            }
        }
    }

    if (db.commit()) {
        accept();
    } else {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось зафиксировать изменения в базе данных.");
    }
}
