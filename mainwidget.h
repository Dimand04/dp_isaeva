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
    MainWidget(int m_userId, QWidget *parent = nullptr);
    ~MainWidget();

private slots:
    void sw_main_change(int index);

    void logout(); // выход из аккаунта
    void loadUserInfo(); // загрузка ника пользователя
    void loadAdminUsers(); // загрузка списка пользователей
    void filterAdminUsers(const QString &searchText); // поиск по пользователям
    void loadAdminRoles(); // загрузка списка ролей
    void filterAdminRoles(const QString &searchText); // поиск по ролям
    void openRoleEditor(int roleId = -1); // вызов roleeditordialog
    void openUserEditor(int targetUserId = -1); // вызов usereditordialog

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
    void fillDemoAdminPermissions();
    void fillDemoNSITypes();
    void fillDemoNSIMaterials();
    void fillDemoDashboard();
    //

    void on_lw_admin_roles_itemSelectionChanged(); // выбор роли

    void on_pb_admin_role_edit_clicked(); // редактирование роли

    void on_pb_admin_role_delete_clicked(); // удаление роли

    void on_lw_admin_users_itemSelectionChanged(); // выбор пользователя

    void on_pb_admin_user_edit_clicked(); // редактирование пользователя

private:
    Ui::MainWidget *ui;
    int m_userId;
};
#endif // MAINWIDGET_H
