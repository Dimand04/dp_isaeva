#ifndef PROJECTESTIMATEDIALOG_H
#define PROJECTESTIMATEDIALOG_H

#include <QDialog>

namespace Ui {
class ProjectEstimateDialog;
}

class ProjectEstimateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectEstimateDialog(int projectId, int estimateId = -1, QWidget *parent = nullptr);
    ~ProjectEstimateDialog();

private slots:
    void onGroupTypeChanged(int index);
    void onMaterialChanged(int index);
    void calculateTotal();
    void on_pb_save_clicked();

private:
    Ui::ProjectEstimateDialog *ui;
    int m_projectId;
    int m_estimateId;

    void setupConnections();
    void loadGroupTypes();
    void loadMaterials();
    void fetchMaterialPrice();
};

#endif // PROJECTESTIMATEDIALOG_H
