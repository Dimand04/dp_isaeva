#ifndef PAYMENTDIALOG_H
#define PAYMENTDIALOG_H

#include <QDialog>

namespace Ui {
class PaymentDialog;
}

class PaymentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PaymentDialog(int clientId, QWidget *parent = nullptr);
    ~PaymentDialog();

private slots:
    void on_pb_save_clicked();

private:
    Ui::PaymentDialog *ui;
    int m_clientId;
};

#endif // PAYMENTDIALOG_H
