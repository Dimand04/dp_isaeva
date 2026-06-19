#ifndef ESTIMATEDIALOG_H
#define ESTIMATEDIALOG_H

#include <QDialog>
#include <QList>
#include "baseeditoritem.h"
#include <QHeaderView>

namespace Ui {
class EstimateDialog;
}

class EstimateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EstimateDialog(const QList<BaseEditorItem*> &items, QWidget *parent = nullptr);
    ~EstimateDialog();

private slots:
    void on_btn_close_clicked();

private:
    Ui::EstimateDialog *ui;

    struct EstimateRow {
        QString name;
        QString unit;
        double quantity = 0.0;
        double price = 0.0;
    };

    void calculateEstimate(const QList<BaseEditorItem*> &items);
    double extractQuantityByUnit(BaseEditorItem *item, const QString &unit);
};

#endif // ESTIMATEDIALOG_H
