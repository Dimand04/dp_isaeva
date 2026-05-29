#include "clienteditordialog.h"
#include "ui_clienteditordialog.h"

ClientEditorDialog::ClientEditorDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClientEditorDialog)
{
    ui->setupUi(this);
}

ClientEditorDialog::~ClientEditorDialog()
{
    delete ui;
}
