#ifndef ROLEEDITORDIALOG_H
#define ROLEEDITORDIALOG_H

#include <QDialog>

namespace Ui {
class RoleEditorDialog;
}

class RoleEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RoleEditorDialog(const int& m_roleId, QWidget *parent = nullptr);
    ~RoleEditorDialog();

private slots:
    void loadPermissions();

private:
    Ui::RoleEditorDialog *ui;
    int m_roleId;
};

#endif // ROLEEDITORDIALOG_H
