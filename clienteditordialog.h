#ifndef CLIENTEDITORDIALOG_H
#define CLIENTEDITORDIALOG_H

#include <QDialog>

namespace Ui {
class ClientEditorDialog;
}

class ClientEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClientEditorDialog(int clientId = -1, QWidget *parent = nullptr);
    ~ClientEditorDialog();

private slots:
    void updatePlaceholders();

    void on_pb_save_clicked();

private:
    Ui::ClientEditorDialog *ui;
    int m_clientId;
};

#endif // CLIENTEDITORDIALOG_H
