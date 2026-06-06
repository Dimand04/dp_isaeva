#ifndef PROJECTSTAGEDIALOG_H
#define PROJECTSTAGEDIALOG_H

#include <QDialog>

namespace Ui {
class ProjectStageDialog;
}

class ProjectStageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectStageDialog(int projectId, int stageId = -1, QWidget *parent = nullptr);
    ~ProjectStageDialog();

private slots:
    void on_pb_save_clicked();
    void updateFactDateState();

private:
    Ui::ProjectStageDialog *ui;
    int m_projectId;
    int m_stageId;

    void loadStatusesToComboBox();
    void loadResponsibleUsers();
};

#endif // PROJECTSTAGEDIALOG_H
