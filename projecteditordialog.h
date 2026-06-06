#ifndef PROJECTEDITORDIALOG_H
#define PROJECTEDITORDIALOG_H

#include <QDialog>

namespace Ui {
class ProjectEditorDialog;
}

class ProjectEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectEditorDialog(int projectId = -1, QWidget *parent = nullptr);
    ~ProjectEditorDialog();

public slots:
    void setClient(int clientId);

private slots:
    void on_pb_save_clicked();

private:
    Ui::ProjectEditorDialog *ui;
    int m_projectId;

    void loadClientsToComboBox();
    void loadStatusesToComboBox();
};

#endif // PROJECTEDITORDIALOG_H
