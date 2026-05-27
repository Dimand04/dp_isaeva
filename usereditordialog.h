#ifndef USEREDITORDIALOG_H
#define USEREDITORDIALOG_H

#include <QDialog>

namespace Ui {
class UserEditorDialog;
}

class UserEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserEditorDialog(int userId, QWidget *parent = nullptr);
    ~UserEditorDialog();

private slots:
    void loadRoles();

    void on_pb_ok_clicked();

private:
    Ui::UserEditorDialog *ui;
    int m_userId;
};

#endif // USEREDITORDIALOG_H
