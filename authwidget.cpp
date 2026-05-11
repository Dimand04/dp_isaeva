#include "authwidget.h"
#include "ui_authwidget.h"

extern int userId;

authWidget::authWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::authWidget)
    , m_userId(-1)
{
    ui->setupUi(this);

    setWindowTitle("Авторизация");

    connect(ui->pb_login, &QPushButton::clicked, this, &authWidget::tryLogin);

    loadUsers();
}

authWidget::~authWidget()
{
    delete ui;
}

bool authWidget::checkAutoLogin()
{
    QSettings settings(qApp->applicationDirPath() + "/config.ini", QSettings::IniFormat);
    bool stayIn = settings.value("auth/staySignedIn", false).toBool();
    int savedId = settings.value("auth/savedUserId", -1).toInt();

    if (stayIn && savedId != -1) {
        QSqlDatabase db = QSqlDatabase::database();
        QSqlQuery query(db);
        query.prepare("SELECT id FROM users WHERE id = :id");
        query.bindValue(":id", savedId);

        if (query.exec() && query.next()) {
            this->m_userId = savedId;
            return true;
        }
    }
    return false;
}

void authWidget::loadUsers()
{
    ui->cb_users->clear();

    ui->cb_users->addItem("Выберите пользователя", 0);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    if (query.exec("SELECT id, login FROM users ORDER BY login ASC")) {
        while (query.next()) {
            int userIdFromDb = query.value(0).toInt();
            QString login = query.value(1).toString();

            ui->cb_users->addItem(login, userIdFromDb);
        }
    } else {
        qCritical() << "Ошибка загрузки списка пользователей для логов:" << query.lastError().text();
    }

    ui->cb_users->setCurrentIndex(0);
}

void authWidget::tryLogin()
{
    if (ui->cb_users->currentIndex() <= 0) {
        QMessageBox::warning(this, "Ошибка", "Выберите пользователя.");
        return;
    }

    int selectedId = ui->cb_users->currentData().toInt();
    QString password = ui->le_password->text();

    if (password.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите пароль.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("SELECT id FROM auth WHERE id = :id AND user_password = :pass");
    query.bindValue(":id", selectedId);
    query.bindValue(":pass", password);

    if (query.exec()) {
        if (query.next()) {
            m_userId = selectedId;

            QSettings settings(qApp->applicationDirPath() + "/config.ini", QSettings::IniFormat);
            if (ui->ckb_stay_signed_in->isChecked()) {
                settings.setValue("auth/savedUserId", m_userId);
                settings.setValue("auth/staySignedIn", true);
            } else {
                settings.remove("auth/savedUserId");
                settings.setValue("auth/staySignedIn", false);
            }

            ::userId = m_userId;

            accept();
        } else {
            QMessageBox::warning(this, "Ошибка доступа", "Неверный пароль.");
            ui->le_password->clear();
            ui->le_password->setFocus();
        }
    } else {
        qCritical() << "Ошибка SQL:" << query.lastError().text();
        QMessageBox::critical(this, "Ошибка", "Ошибка при проверке данных в БД.");
    }
}
