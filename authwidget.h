#ifndef AUTHWIDGET_H
#define AUTHWIDGET_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

namespace Ui {
class authWidget;
}

class authWidget : public QDialog
{
    Q_OBJECT

public:
    explicit authWidget(QWidget *parent = nullptr);
    ~authWidget();

    int getUserId() const { return userId; }

private slots:
    void loadUsers();
    void tryLogin();

private:
    Ui::authWidget *ui;
    int userId;
};

#endif // AUTHWIDGET_H
