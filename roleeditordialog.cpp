#include "roleeditordialog.h"
#include "ui_roleeditordialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

RoleEditorDialog::RoleEditorDialog(const int& m_roleId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RoleEditorDialog)
    , m_roleId(m_roleId)
{
    ui->setupUi(this);

    loadPermissions();
}

RoleEditorDialog::~RoleEditorDialog()
{
    delete ui;
}

void RoleEditorDialog::loadPermissions()
{
    ui->lw_permissions->clear();

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "База данных не открыта!";
        return;
    }

    QSet<int> allowedModules;

    if (m_roleId != -1) {
        QSqlQuery accessQuery(db);
        accessQuery.prepare("SELECT module_id FROM role_access WHERE role_id = :role_id");
        accessQuery.bindValue(":role_id", m_roleId);

        if (accessQuery.exec()) {
            while (accessQuery.next()) {
                allowedModules.insert(accessQuery.value(0).toInt());
            }
        } else {
            qWarning() << "Ошибка загрузки текущих прав роли:" << accessQuery.lastError().text();
        }
    }

    QSqlQuery query("SELECT id, name FROM modules ORDER BY name ASC", db);
    if (!query.exec()) {
        qCritical() << "Ошибка при загрузке списка модулей:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        int moduleId = query.value(0).toInt();
        QString moduleName = query.value(1).toString();

        QListWidgetItem *item = new QListWidgetItem(moduleName, ui->lw_permissions);

        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

        if (allowedModules.contains(moduleId)) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }

        item->setData(Qt::UserRole, moduleId);
    }
}
