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
    MainWidget(int userId, QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void sw_main_change(int index);

    void logout(); // выход из аккаунта
    void loadUserInfo(); // загрузка ника пользователя

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
    void fillDemoCatalogSpecs();
    void fillDemoCatalogEstimate();
    void setupExportFilters();
    void fillExportPreviewDemo();
    void fillExportFileSelection(int projectIndex);
    void fillDemoAdminUsers();
    void fillDemoAdminPermissions();
    void fillDemoNSITypes();
    void fillDemoNSIMaterials();
    void fillDemoDashboard();
    //
private:
    Ui::MainWidget *ui;
    int userId;
};
#endif // MAINWIDGET_H
