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
    explicit ClientEditorDialog(QWidget *parent = nullptr);
    ~ClientEditorDialog();

private:
    Ui::ClientEditorDialog *ui;
};

#endif // CLIENTEDITORDIALOG_H
