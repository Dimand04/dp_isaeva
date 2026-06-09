#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QCloseEvent>
#include <QTableWidgetItem>

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

protected:
    void closeEvent(QCloseEvent *event) override;

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
    void fillNSITypes(); // отрисовка таблицы справочников
    void loadNsiTable(const QString &tableName, const QString &headerLabel); // загрузка детальной таблицы
    void filterNSITable(const QString &searchText); // поиск по данным
    void openNsiEditor(int itemId = -1); // вызов виджета редактирования справочников
    void loadClientsTable(); // формирование таблицы клиентов
    void filterClientsTable(const QString &searchText); // поиск по клиентам
    void clearClientDetailsUI(); // Очистка всех полей и вкладок
    void loadClientDetails(int clientId); // загрузка данных клиента
    void loadProjectsTable(); // формирование таблицы проектов
    void setupProjectFilters(); // настройка фильтров проектов
    void applyProjectFilters(); // фильтрация списка проектов
    void clearProjectDetailsUI();
    void loadProjectDetails(int projectId);
    void loadClientProjects(int clientId);
    void loadClientFinance(int clientId);
    void loadProjectStages(int projectId);
    void loadProjectEstimates(int projectId);
    void loadProjectFiles(int projectId);
    void loadClientFiles(int clientId);
    void loadHomeDashboard();
    void loadExportData();
    void updateExportPreview();
    void loadCompanySettings();
    void saveCompanySettings();

    // Спорно
    void updateCatalogLayout();

    void on_lw_admin_roles_itemSelectionChanged(); // выбор роли

    void on_pb_admin_role_edit_clicked(); // редактирование роли

    void on_pb_admin_role_delete_clicked(); // удаление роли

    void on_lw_admin_users_itemSelectionChanged(); // выбор пользователя

    void on_pb_admin_user_edit_clicked(); // редактирование пользователя

    void on_lw_nsi_types_itemSelectionChanged(); // выбор справочника

    void on_pb_nsi_edit_clicked(); // редактирование айтема

    void on_tw_nsi_itemSelectionChanged(); // выбор айтема справочника

    void on_pb_nsi_delete_clicked(); // удаление записи справочника

    void on_pb_admin_user_delete_clicked(); // удаление пользователя

    void on_tw_clients_itemSelectionChanged();

    void on_tw_projects_list_itemSelectionChanged();

    void on_pb_client_edit_clicked();

    void on_pb_client_create_clicked();

    void on_tw_client_projects_itemDoubleClicked(QTableWidgetItem *item);

    void on_pb_add_payment_clicked();

    void on_pb_project_create_clicked();

    void on_pb_project_edit_clicked();

    void on_pb_stage_add_clicked();

    void on_pb_stage_edit_clicked();

    void on_pb_stage_delete_clicked();

    void on_tw_project_estimates_itemSelectionChanged();

    void on_pb_estimate_add_clicked();

    void on_pb_estimate_edit_clicked();

    void on_pb_estimate_delete_clicked();

    void on_pb_client_project_add_clicked();

    void on_pb_project_file_add_clicked();

    void on_tw_project_files_itemDoubleClicked(QTableWidgetItem *item);

    void on_pb_project_file_delete_clicked();

    void on_pb_project_open_folder_clicked();

    void on_pb_client_file_add_clicked();

    void on_pb_client_file_delete_clicked();

    void on_tw_client_files_itemDoubleClicked(QTableWidgetItem *item);

    void on_pb_client_open_folder_clicked();

    void on_lb_stat_projects_clicked();

    void on_lb_stat_clients_clicked();

    void on_tw_home_deadlines_itemDoubleClicked(QTableWidgetItem *item);

    void on_lb_new_client_card_clicked();

    void on_lb_new_project_card_clicked();

    void on_cb_export_project_currentIndexChanged(int index);

    void on_cb_export_doc_type_currentIndexChanged(int index);

    void on_pb_export_save_clicked();

    void on_pb_save_settings_clicked();

    void on_pb_select_logo_clicked();

    void on_pb_open_editor_clicked();

private:
    Ui::MainWidget *ui;
    int m_userId;
    QString m_companyLogoPath;
};
#endif // MAINWIDGET_H
