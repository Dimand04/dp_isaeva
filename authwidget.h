#ifndef AUTHWIDGET_H
#define AUTHWIDGET_H

#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QSettings>

namespace Ui {
class authWidget;
}

class authWidget : public QDialog
{
    Q_OBJECT

public:
    explicit authWidget(QWidget *parent = nullptr);
    ~authWidget();

    int getUserId() const { return m_userId; }

    bool checkAutoLogin();

private slots:
    void loadUsers();
    void tryLogin();

private:
    Ui::authWidget *ui;
    int m_userId = -1;
};

#endif // AUTHWIDGET_H
