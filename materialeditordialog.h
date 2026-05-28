#ifndef MATERIALEDITORDIALOG_H
#define MATERIALEDITORDIALOG_H

#include <QDialog>
#include <QComboBox>

namespace Ui {
class MaterialEditorDialog;
}

class MaterialEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MaterialEditorDialog(int materialId = -1, QWidget *parent = nullptr);
    ~MaterialEditorDialog();

private slots:
    void on_pb_save_clicked();

private:
    Ui::MaterialEditorDialog *ui;
    int m_materialId;

    void loadComboBox(QComboBox *cb, const QString &tableName, const QString &defaultText);
    void loadDictionaries();
};

#endif // MATERIALEDITORDIALOG_H
