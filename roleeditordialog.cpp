#include "roleeditordialog.h"
#include "ui_roleeditordialog.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

RoleEditorDialog::RoleEditorDialog(int roleId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RoleEditorDialog)
    , m_roleId(roleId)
{
    ui->setupUi(this);

    if (m_roleId == -1) {
        setWindowTitle("Создание новой роли");
        ui->pb_ok->setText("Создать");
        ui->le_name->clear();
    } else {
        setWindowTitle("Редактирование роли");
        ui->pb_ok->setText("Обновить");

        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            QSqlQuery query(db);
            query.prepare("SELECT name FROM roles WHERE id = :id");
            query.bindValue(":id", m_roleId);
            if (query.exec() && query.next()) {
                ui->le_name->setText(query.value(0).toString());
            } else {
                qWarning() << "Ошибка при загрузке названия роли:" << query.lastError().text();
            }
        }
    }

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

void RoleEditorDialog::on_pb_ok_clicked()
{
    QString roleName = ui->le_name->text().trimmed();
    if (roleName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Название роли не может быть пустым.");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();

    if (!db.transaction()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось начать транзакцию базы данных.");
        return;
    }

    QSqlQuery query(db);
    int targetRoleId = m_roleId;

    if (m_roleId == -1) {
        query.prepare("INSERT INTO roles (name) VALUES (:name)");
        query.bindValue(":name", roleName);

        if (!query.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Ошибка", "Не удалось создать роль:\n" + query.lastError().text());
            return;
        }
        targetRoleId = query.lastInsertId().toInt();

    } else {
        query.prepare("UPDATE roles SET name = :name WHERE id = :id");
        query.bindValue(":name", roleName);
        query.bindValue(":id", targetRoleId);

        if (!query.exec()) {
            db.rollback();
            QMessageBox::critical(this, "Ошибка", "Не удалось обновить роль:\n" + query.lastError().text());
            return;
        }
    }

    QSqlQuery accessQuery(db);
    accessQuery.prepare("DELETE FROM role_access WHERE role_id = :role_id");
    accessQuery.bindValue(":role_id", targetRoleId);
    if (!accessQuery.exec()) {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось очистить старые права:\n" + accessQuery.lastError().text());
        return;
    }

    accessQuery.prepare("INSERT INTO role_access (role_id, module_id) VALUES (:role_id, :module_id)");

    for (int i = 0; i < ui->lw_permissions->count(); ++i) {
        QListWidgetItem *item = ui->lw_permissions->item(i);

        if (item->checkState() == Qt::Checked) {
            int moduleId = item->data(Qt::UserRole).toInt();

            accessQuery.bindValue(":role_id", targetRoleId);
            accessQuery.bindValue(":module_id", moduleId);

            if (!accessQuery.exec()) {
                db.rollback();
                QMessageBox::critical(this, "Ошибка", "Не удалось сохранить права доступа:\n" + accessQuery.lastError().text());
                return;
            }
        }
    }

    if (db.commit()) {
        accept();
    } else {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось зафиксировать изменения.");
    }
}
