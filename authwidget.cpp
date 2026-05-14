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
    QString savedToken = settings.value("auth/token", "").toString();

    if (stayIn && !savedToken.isEmpty()) {
        QSqlDatabase db = QSqlDatabase::database();
        if (!db.isOpen()) return false;

        QString hashedToken = hashToken(savedToken);

        QSqlQuery query(db);
        query.prepare("SELECT id FROM auth WHERE remember_token = :token");
        query.bindValue(":token", hashedToken);

        if (query.exec() && query.next()) {
            this->m_userId = query.value(0).toInt();
            return true;
        } else {
            settings.remove("auth/token");
            settings.setValue("auth/staySignedIn", false);
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
                QString rawToken = generateSecureToken();
                QString hashedToken = hashToken(rawToken);

                QSqlQuery updateQuery(db);
                updateQuery.prepare("UPDATE auth SET remember_token = :token WHERE id = :id");
                updateQuery.bindValue(":token", hashedToken);
                updateQuery.bindValue(":id", m_userId);

                if (!updateQuery.exec()) {
                    qWarning() << "Не удалось сохранить токен в БД:" << updateQuery.lastError().text();
                }

                settings.setValue("auth/token", rawToken);
                settings.setValue("auth/staySignedIn", true);
            } else {
                QSqlQuery updateQuery(db);
                updateQuery.prepare("UPDATE auth SET remember_token = NULL WHERE id = :id");
                updateQuery.bindValue(":id", m_userId);
                updateQuery.exec();

                settings.remove("auth/token");
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

QString authWidget::generateSecureToken()
{
    QByteArray randomBytes;
    for (int i = 0; i < 32; ++i) {
        randomBytes.append(static_cast<char>(QRandomGenerator::global()->bounded(256)));
    }
    return QString(randomBytes.toHex());
}

QString authWidget::hashToken(const QString &token)
{
    return QString(QCryptographicHash::hash(token.toUtf8(), QCryptographicHash::Sha256).toHex());
}
