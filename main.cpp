#include "mainwidget.h"
#include <QSqlDatabase>
#include <QMessageBox>
#include <QApplication>
#include "authwidget.h"

int userId = -1;

bool setupDatabaseConnection() {
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
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
    int currentExitCode = 0;

    do {
        QApplication a(argc, argv);

        if (!setupDatabaseConnection()) {
            return 1;
        }

        authWidget auth;
        int targetUserId = -1;

        if (auth.checkAutoLogin()) {
            targetUserId = auth.getUserId();
        } else if (auth.exec() == QDialog::Accepted) {
            targetUserId = auth.getUserId();
        }

        if (targetUserId != -1) {
            ::userId = targetUserId;

            MainWidget w(targetUserId);
            w.show();

            currentExitCode = a.exec();
        } else {
            currentExitCode = 0;
        }

    } while (currentExitCode == 1000);

    return currentExitCode;
}
