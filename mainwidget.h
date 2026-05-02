#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWidget;
}
QT_END_NAMESPACE

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void sw_main_change(int index);

    // fake
    void fillDemoClients();
    void fillDemoClientDetails();
    void fillDemoClientProjects();
    void fillDemoClientFinance();
    void fillDemoClientFiles();
    void setupProjectFilters();
    void fillDemoProjectsTable();
    void fillDemoProjectDetail();
    void fillDemoProjectEstimate();
    void fillDemoProjectFiles();
    void setupCatalogFilters();
    void fillDemoCatalog();
    void updateCatalogLayout();
    //
private:
    Ui::MainWidget *ui;
};
#endif // MAINWIDGET_H
