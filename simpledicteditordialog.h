#ifndef SIMPLEDICTEDITORDIALOG_H
#define SIMPLEDICTEDITORDIALOG_H

#include <QDialog>

namespace Ui {
class SimpleDictEditorDialog;
}

class SimpleDictEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SimpleDictEditorDialog(const QString &tableName, const QString &windowTitle, int itemId = -1, QWidget *parent = nullptr);
    ~SimpleDictEditorDialog();

private slots:
    void on_pb_save_clicked();

private:
    Ui::SimpleDictEditorDialog *ui;
    QString m_tableName;
    int m_itemId;
};

#endif // SIMPLEDICTEDITORDIALOG_H
