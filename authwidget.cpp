#include "authwidget.h"
#include "ui_authwidget.h"

authWidget::authWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::authWidget)
{
    ui->setupUi(this);

    setWindowTitle("Авторизация");

    loadUsers();
}

authWidget::~authWidget()
{
    delete ui;
}

void authWidget::loadUsers()
{
    ui->cb_users->clear();

    ui->cb_users->addItem("Выберите пользователя", 0);

    QSqlDatabase db = QSqlDatabase::database("db_dp_isaeva");
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

}
