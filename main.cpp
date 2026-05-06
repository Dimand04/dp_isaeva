#include "mainwidget.h"
#include <QSqlDatabase>
#include <QMessageBox>
#include <QApplication>
#include "authwidget.h"

bool setupDatabaseConnection() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "db_dp_isaeva");
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("db_dp_isaeva");
    db.setUserName("root");
    db.setPassword("1234");

    if (!db.open()) {
        QMessageBox::critical(nullptr, "Ошибка подключения", "Не удалось подключиться к базе данных!");
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!setupDatabaseConnection())
        return 1;

    authWidget auth;
    if (auth.exec() == QDialog::Accepted) {
        int userId = auth.getUserId();

        MainWidget w(userId);
        w.show();
        return a.exec();
    }

    return 0;
}
