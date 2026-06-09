#include "mainwidget.h"
#include "ui_mainwidget.h"
#include <QMessageBox>
#include <QSettings>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "roleeditordialog.h"
#include "usereditordialog.h"
#include "simpledicteditordialog.h"
#include "materialeditordialog.h"
#include <QDate>
#include "clienteditordialog.h"
#include "paymentdialog.h"
#include "projecteditordialog.h"
#include "projectstagedialog.h"
#include "projectestimatedialog.h"
#include <QFileDialog>
#include "filestoragemanager.h"
#include <QDesktopServices>
#include <QPdfWriter>
#include "editorwindow.h"

MainWidget::MainWidget(int userId, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
    , m_userId(m_userId)
{
    ui->setupUi(this);

    ui->splitter->setSizes({150, 800});

    connect(ui->lw_main, &QListWidget::currentRowChanged, this, &MainWidget::sw_main_change);
    connect(ui->splitter_4, &QSplitter::splitterMoved, this, &MainWidget::updateCatalogLayout);
    connect(ui->pb_logout, &QPushButton::clicked, this, &MainWidget::logout);
    connect(ui->le_adminUserSearch, &QLineEdit::textChanged, this, &MainWidget::filterAdminUsers);
    connect(ui->le_adminRoleSearch, &QLineEdit::textChanged, this, &MainWidget::filterAdminRoles);
    connect(ui->pb_admin_role_create, &QPushButton::clicked, this, [this]() {
        openRoleEditor(-1);
    });
    connect(ui->pb_admin_user_create, &QPushButton::clicked, this, [this]() {
        openUserEditor(-1);
    });
    connect(ui->le_nsi_search, &QLineEdit::textChanged, this, &MainWidget::filterNSITable);
    connect(ui->pb_nsi_add, &QPushButton::clicked, this, [this]() {
        openNsiEditor(-1);
    });
    connect(ui->le_clients_search, &QLineEdit::textChanged, this, &MainWidget::filterClientsTable);
    connect(ui->le_projects_search, &QLineEdit::textChanged, this, &MainWidget::applyProjectFilters);
    connect(ui->lb_proj_client_ref, &QLabel::linkActivated, this, [this](const QString &linkData){
        int clientId = linkData.toInt();

        ui->lw_main->setCurrentRow(1);
        sw_main_change(1);

        for (int row = 0; row < ui->tw_clients->rowCount(); ++row) {
            QTableWidgetItem *item = ui->tw_clients->item(row, 0);

            if (item && item->data(Qt::UserRole).toInt() == clientId) {
                ui->tw_clients->setCurrentItem(item);

                ui->tw_clients->scrollToItem(item);

                break;
            }
        }
    });

    ui->lw_main->setCurrentRow(0);
    loadUserInfo();

    setupProjectFilters();
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение выхода",
                                  "Вы действительно хотите закрыть программу?\nНесохраненные данные могут быть потеряны.",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWidget::sw_main_change(int index)
{
    if (index < 0) return;

    ui->sw_main->setCurrentIndex(index);

    switch (index) {
    case 0:
        qDebug() << "Главная";

        loadHomeDashboard();

        break;

    case 1:
        qDebug() << "Клиенты";

        loadClientsTable();
        ui->splitter_2->setSizes({1, 0});

        break;

    case 2:
        qDebug() << "Проекты";

        loadProjectsTable();
        ui->splitter_5->setSizes({1, 0});

        break;

    case 3:
        qDebug() << "Каталог";
        break;

    case 4:
        qDebug() << "Экспорт";

        loadExportData();

        break;

    case 5:
        qDebug() << "Администрирование";

        ui->pb_admin_role_create->setEnabled(true);
        ui->pb_admin_role_edit->setEnabled(false);
        ui->pb_admin_role_delete->setEnabled(false);

        ui->pb_admin_user_create->setEnabled(true);
        ui->pb_admin_user_edit->setEnabled(false);
        ui->pb_admin_user_delete->setEnabled(false);

        ui->pb_nsi_add->setEnabled(true);
        ui->pb_nsi_edit->setEnabled(false);
        ui->pb_nsi_delete->setEnabled(false);

        loadAdminUsers();
        loadAdminRoles();
        fillNSITypes();
        loadCompanySettings();
        ui->splitter_8->setSizes({1, 0});

        break;

    default:
        break;
    }
}

void MainWidget::logout()
{
    auto result = QMessageBox::question(this, "Выход", "Вы действительно хотите выйти из учетной записи?");
    if (result == QMessageBox::No) return;

    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("UPDATE auth SET remember_token = NULL WHERE id = :id");
        query.bindValue(":id", m_userId);

        if (!query.exec()) {
            qWarning() << "Ошибка при удалении токена из БД:" << query.lastError().text();
        }
    }

    QSettings settings(qApp->applicationDirPath() + "/config.ini", QSettings::IniFormat);

    settings.beginGroup("auth");
    settings.setValue("staySignedIn", false);
    settings.remove("token");
    settings.remove("savedUserId");
    settings.endGroup();

    settings.sync();

    qApp->exit(1000);
}

void MainWidget::loadUserInfo()
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare("SELECT login FROM users where id = ?");
    query.addBindValue(m_userId);
    if (query.exec()) {
        if (query.next()) {
            QString login = query.value(0).toString();
            ui->lb_nick->setText(login);
        }
    } else {
        qCritical() << "Ошибка загрузки информации о пользователе:" << query.lastError().text();
    }
}

void MainWidget::setupProjectFilters()
{
    ui->cb_project_status_filter->blockSignals(true);

    ui->cb_project_status_filter->clear();

    ui->cb_project_status_filter->addItem("📊 Все статусы", "Все");
    ui->cb_project_status_filter->addItem("✏️ Проектирование", "Проектирование");
    ui->cb_project_status_filter->addItem("🏗️ Строительство", "Строительство");
    ui->cb_project_status_filter->addItem("✅ Завершено", "Завершено");
    ui->cb_project_status_filter->addItem("❄️ Заморожено", "Заморожено");

    ui->cb_project_status_filter->blockSignals(false);

    connect(ui->cb_project_status_filter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWidget::applyProjectFilters);
}

void MainWidget::applyProjectFilters()
{
    QString searchText = ui->le_projects_search->text();

    QString targetStatus = ui->cb_project_status_filter->currentData().toString();

    for (int row = 0; row < ui->tw_projects_list->rowCount(); ++row) {

        bool matchStatus = true;

        if (targetStatus != "Все") {
            QTableWidgetItem *statusItem = ui->tw_projects_list->item(row, 2);

            if (statusItem && statusItem->text() != targetStatus) {
                matchStatus = false;
            }
        }

        bool matchText = true;

        if (matchStatus && !searchText.isEmpty()) {
            matchText = false;

            for (int col = 0; col < ui->tw_projects_list->columnCount(); ++col) {
                QTableWidgetItem *item = ui->tw_projects_list->item(row, col);
                if (item && item->text().contains(searchText, Qt::CaseInsensitive)) {
                    matchText = true;
                    break;
                }
            }

            if (!matchText) {
                QTableWidgetItem *nameItem = ui->tw_projects_list->item(row, 0);
                if (nameItem && nameItem->data(Qt::UserRole + 1).toString().contains(searchText, Qt::CaseInsensitive)) {
                    matchText = true;
                }
            }
        }

        ui->tw_projects_list->setRowHidden(row, !(matchStatus && matchText));
    }
}

void MainWidget::updateCatalogLayout()
{
    if (!ui->gridLayout_catalog) return;

    // 1. Сначала извлекаем все существующие карточки из сетки в список
    QList<QWidget*> cards;
    QLayoutItem *item;
    while ((item = ui->gridLayout_catalog->takeAt(0)) != nullptr) {
        if (QWidget *w = item->widget()) {
            cards.append(w);
        }
        delete item; // Удаляем только элемент лейаута, не сам виджет
    }

    // 2. Считаем количество колонок
    int availableWidth = ui->scrollArea_catalog->viewport()->width();
    int cardWidth = 210;
    int spacing = 0;
    int columns = qMax(1, (availableWidth - 20) / (cardWidth + spacing));

    // 3. Расставляем карточки заново из нашего списка
    for (int i = 0; i < cards.size(); ++i) {
        int row = i / columns;
        int col = i % columns;

        // Добавляем карточку с выравниванием по левому верхнему углу,
        // чтобы они не растягивались внутри ячеек
        ui->gridLayout_catalog->addWidget(cards[i], row, col, Qt::AlignLeft | Qt::AlignTop);
    }

    // 4. Сбрасываем старые растяжения и ставим новые
    for(int i = 0; i < ui->gridLayout_catalog->columnCount(); ++i)
        ui->gridLayout_catalog->setColumnStretch(i, 0);

    // Пружина справа (в колонку за последней карточкой)
    ui->gridLayout_catalog->setColumnStretch(columns, 1);

    // Пружина снизу
    ui->gridLayout_catalog->setRowStretch(ui->gridLayout_catalog->rowCount(), 1);
}

void MainWidget::loadAdminUsers()
{
    ui->lw_admin_users->blockSignals(true);

    ui->lw_admin_users->clear();
    ui->lw_admin_users->setIconSize(QSize(32, 32));

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "База данных не открыта!";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT u.id, u.login, r.name "
                  "FROM users u "
                  "LEFT JOIN roles r ON u.role_id = r.id "
                  "ORDER BY u.login ASC");

    if (!query.exec()) {
        qCritical() << "Ошибка при загрузке списка пользователей:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        int userId = query.value(0).toInt();
        QString login = query.value(1).toString();
        QString roleName = query.value(2).toString();

        if (roleName.isEmpty()) {
            roleName = "Роль не назначена";
        }

        bool isAdmin = roleName.contains("Администратор", Qt::CaseInsensitive);

        QListWidgetItem *item = new QListWidgetItem();
        item->setText(login + "\n" + roleName);

        QIcon icon = style()->standardIcon(isAdmin ? QStyle::SP_ComputerIcon : QStyle::SP_DirHomeIcon);
        item->setIcon(icon);

        item->setData(Qt::UserRole, userId);

        ui->lw_admin_users->addItem(item);
    }

    ui->lw_admin_users->blockSignals(false);
}

void MainWidget::filterAdminUsers(const QString &searchText)
{
    for (int i = 0; i < ui->lw_admin_users->count(); ++i) {
        QListWidgetItem *item = ui->lw_admin_users->item(i);

        bool match = item->text().contains(searchText, Qt::CaseInsensitive);

        item->setHidden(!match);
    }
}

void MainWidget::loadAdminRoles()
{
    ui->lw_admin_roles->blockSignals(true);

    ui->lw_admin_roles->clear();
    ui->lw_admin_roles->setIconSize(QSize(32, 32));

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "База данных не открыта!";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id, name FROM roles ORDER BY name ASC");

    if (!query.exec()) {
        qCritical() << "Ошибка при загрузке списка ролей:" << query.lastError().text();
        return;
    }

    while (query.next()) {
        int roleId = query.value(0).toInt();
        QString roleName = query.value(1).toString();

        QListWidgetItem *item = new QListWidgetItem();
        item->setText(roleName);

        QIcon icon = style()->standardIcon(QStyle::SP_FileDialogDetailedView);
        item->setIcon(icon);

        item->setData(Qt::UserRole, roleId);

        ui->lw_admin_roles->addItem(item);
    }

    ui->lw_admin_roles->blockSignals(false);
}

void MainWidget::filterAdminRoles(const QString &searchText)
{
    for (int i = 0; i < ui->lw_admin_roles->count(); ++i) {
        QListWidgetItem *item = ui->lw_admin_roles->item(i);

        bool match = item->text().contains(searchText, Qt::CaseInsensitive);

        item->setHidden(!match);
    }
}

void MainWidget::openRoleEditor(int roleId)
{
    RoleEditorDialog dialog(roleId, this);

    if (dialog.exec() == QDialog::Accepted) {

        loadAdminRoles();

        if (roleId == -1) {
            QMessageBox::information(this, "Успех", "Новая роль успешно создана!");
        } else {
            QMessageBox::information(this, "Успех", "Права и название роли успешно обновлены!");
        }
    }
}

void MainWidget::openUserEditor(int targetUserId)
{
    UserEditorDialog dialog(targetUserId, this);

    if (dialog.exec() == QDialog::Accepted) {

        loadAdminUsers();

        if (targetUserId == -1) {
            QMessageBox::information(this, "Успех", "Новый пользователь успешно создан!");
        } else {
            QMessageBox::information(this, "Успех", "Данные пользователя успешно обновлены!");
        }
    }
}

void MainWidget::fillNSITypes()
{
    ui->lw_nsi_types->blockSignals(true);

    ui->lw_nsi_types->clear();
    ui->lw_nsi_types->setIconSize(QSize(24, 24));

    struct TypeDemo {
        QString name;
        QIcon icon;
        QString systemKey;
    };

    QList<TypeDemo> types = {
        {"Категории материалов", style()->standardIcon(QStyle::SP_DirIcon), "sys_categories"},
        {"Материалы", style()->standardIcon(QStyle::SP_FileIcon), "sys_materials"},
        {"Единицы измерения", style()->standardIcon(QStyle::SP_FileDialogContentsView), "sys_units"}
    };

    for (const auto &t : types) {
        QListWidgetItem *item = new QListWidgetItem(t.icon, t.name);

        item->setData(Qt::UserRole, t.systemKey);

        ui->lw_nsi_types->addItem(item);
    }

    ui->lw_nsi_types->blockSignals(false);
}

void MainWidget::on_lw_admin_roles_itemSelectionChanged()
{
    bool hasSelection = (ui->lw_admin_roles->currentItem() != nullptr);
    ui->pb_admin_role_edit->setEnabled(hasSelection);
    ui->pb_admin_role_delete->setEnabled(hasSelection);
}

void MainWidget::on_pb_admin_role_edit_clicked()
{
    QListWidgetItem *selectedItem = ui->lw_admin_roles->currentItem();

    if (!selectedItem) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, выберите роль из списка для редактирования.");
        return;
    }

    int roleId = selectedItem->data(Qt::UserRole).toInt();

    openRoleEditor(roleId);
}

void MainWidget::on_pb_admin_role_delete_clicked()
{
    QListWidgetItem *selectedItem = ui->lw_admin_roles->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "Внимание", "Выберите роль для удаления.");
        return;
    }

    int roleId = selectedItem->data(Qt::UserRole).toInt();
    QString roleName = selectedItem->text();

    if (roleId == 1) {
        QMessageBox::critical(this, "Ошибка", "Базовую роль Администратора удалить нельзя!");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();

    QSqlQuery checkQuery(db);
    checkQuery.prepare("SELECT COUNT(id) FROM users WHERE role_id = :role_id");
    checkQuery.bindValue(":role_id", roleId);

    if (checkQuery.exec() && checkQuery.next()) {
        int usersCount = checkQuery.value(0).toInt();

        if (usersCount > 0) {
            QMessageBox::warning(this, "Удаление запрещено",
                                 QString("Невозможно удалить роль «%1», так как она назначена %2 пользователям.\n\n"
                                         "Сначала измените роли у этих пользователей.")
                                     .arg(roleName).arg(usersCount));
            return;
        }
    }

    auto reply = QMessageBox::question(this, "Подтверждение",
                                       QString("Вы действительно хотите безвозвратно удалить роль «%1»?").arg(roleName),
                                       QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) return;

    db.transaction();

    QSqlQuery deleteAccess(db);
    deleteAccess.prepare("DELETE FROM role_access WHERE role_id = :role_id");
    deleteAccess.bindValue(":role_id", roleId);
    if (!deleteAccess.exec()) {
        db.rollback();
        qWarning() << "Ошибка удаления прав:" << deleteAccess.lastError().text();
        return;
    }

    QSqlQuery deleteRole(db);
    deleteRole.prepare("DELETE FROM roles WHERE id = :id");
    deleteRole.bindValue(":id", roleId);

    if (deleteRole.exec()) {
        db.commit();
        QMessageBox::information(this, "Успех", "Роль успешно удалена.");
        loadAdminRoles();
    } else {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось удалить роль:\n" + deleteRole.lastError().text());
    }
}

void MainWidget::on_pb_admin_user_edit_clicked()
{
    QListWidgetItem *selectedItem = ui->lw_admin_users->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "Внимание", "Выберите пользователя для редактирования.");
        return;
    }

    int targetUserId = selectedItem->data(Qt::UserRole).toInt();

    openUserEditor(targetUserId);
}

void MainWidget::on_lw_admin_users_itemSelectionChanged()
{
    bool hasSelection = (ui->lw_admin_users->currentItem() != nullptr);
    ui->pb_admin_user_edit->setEnabled(hasSelection);
    ui->pb_admin_user_delete->setEnabled(hasSelection);
}

void MainWidget::on_lw_nsi_types_itemSelectionChanged()
{
    QListWidgetItem *selectedItem = ui->lw_nsi_types->currentItem();
    if (!selectedItem) return;

    QString sysKey = selectedItem->data(Qt::UserRole).toString();

    if (sysKey == "sys_categories") {
        loadNsiTable("categories", "Название категории");

    } else if (sysKey == "sys_materials") {
        loadNsiTable("materials", "Название материала");

    } else if (sysKey == "sys_suppliers") {
        loadNsiTable("suppliers", "Наименование поставщика");

    } else if (sysKey == "sys_units") {
        loadNsiTable("units", "Единица измерения");

    } else {
        qDebug() << "Раздел находится в разработке:" << sysKey;

        ui->splitter_8->setSizes({1, 0});
        return;
    }

    ui->splitter_8->setSizes({1, 1});
}

void MainWidget::loadNsiTable(const QString &tableName, const QString &headerLabel)
{
    ui->tw_nsi->blockSignals(true);

    ui->tw_nsi->clear();
    ui->tw_nsi->setColumnCount(1);

    ui->tw_nsi->setHorizontalHeaderLabels(QStringList() << headerLabel);

    ui->tw_nsi->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_nsi->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_nsi->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tw_nsi->verticalHeader()->setVisible(false);

    ui->tw_nsi->setRowCount(0);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "База данных не открыта!";
        return;
    }

    QSqlQuery query(db);
    QString sql = QString("SELECT id, name FROM %1 ORDER BY name ASC").arg(tableName);
    query.prepare(sql);

    if (!query.exec()) {
        qCritical() << "Ошибка при загрузке справочника" << tableName << ":" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        int itemId = query.value(0).toInt();
        QString itemName = query.value(1).toString();

        ui->tw_nsi->insertRow(row);

        QTableWidgetItem *nameItem = new QTableWidgetItem(itemName);
        nameItem->setData(Qt::UserRole, itemId);

        ui->tw_nsi->setItem(row, 0, nameItem);
        row++;
    }

    ui->tw_nsi->blockSignals(false);
}

void MainWidget::filterNSITable(const QString &searchText)
{
    for (int row = 0; row < ui->tw_nsi->rowCount(); ++row) {

        QTableWidgetItem *item = ui->tw_nsi->item(row, 0);

        if (item) {
            bool match = item->text().contains(searchText, Qt::CaseInsensitive);

            ui->tw_nsi->setRowHidden(row, !match);
        }
    }
}

void MainWidget::openNsiEditor(int itemId)
{
    QListWidgetItem *selectedDict = ui->lw_nsi_types->currentItem();
    if (!selectedDict) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, выберите справочник слева.");
        return;
    }

    QString sysKey = selectedDict->data(Qt::UserRole).toString();

    if (sysKey == "sys_categories") {

        QString title = (itemId == -1) ? "Создание категории" : "Редактирование категории";
        SimpleDictEditorDialog dialog("categories", title, itemId, this);

        if (dialog.exec() == QDialog::Accepted) {
            loadNsiTable("categories", "Название категории");

            if (itemId == -1) {
                QMessageBox::information(this, "Успех", "Новая категория успешно создана!");
            } else {
                QMessageBox::information(this, "Успех", "Категория успешно обновлена!");
            }
        }

    } else if (sysKey == "sys_units") {

        QString title = (itemId == -1) ? "Создание единицы измерения" : "Редактирование единицы измерения";
        SimpleDictEditorDialog dialog("units", title, itemId, this);

        if (dialog.exec() == QDialog::Accepted) {
            loadNsiTable("units", "Единица измерения");

            if (itemId == -1) {
                QMessageBox::information(this, "Успех", "Новая единица измерения успешно создана!");
            } else {
                QMessageBox::information(this, "Успех", "Единица измерения успешно обновлена!");
            }
        }

    } else if (sysKey == "sys_materials") {

        MaterialEditorDialog dialog(itemId, this);

        if (dialog.exec() == QDialog::Accepted) {

            loadNsiTable("materials", "Название материала");

            if (itemId == -1) {
                QMessageBox::information(this, "Успех", "Новый материал успешно создан!");
            } else {
                QMessageBox::information(this, "Успех", "Данные материала успешно обновлены!");
            }
        }
    }
}

void MainWidget::loadClientsTable()
{
    ui->tw_clients->blockSignals(true);

    ui->tw_clients->clear();
    ui->tw_clients->setColumnCount(2);
    ui->tw_clients->setHorizontalHeaderLabels(QStringList() << "ФИО / Организация" << "Номер телефона");

    ui->tw_clients->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_clients->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_clients->verticalHeader()->setVisible(false);

    ui->tw_clients->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_clients->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tw_clients->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->tw_clients->setRowCount(0);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "База данных не открыта!";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id, name, phone, type FROM clients ORDER BY name ASC");

    if (!query.exec()) {
        qCritical() << "Ошибка при загрузке клиентов:" << query.lastError().text();
        return;
    }

    int row = 0;
    while (query.next()) {
        int clientId = query.value(0).toInt();
        QString name = query.value(1).toString();
        QString phone = query.value(2).toString();
        QString type = query.value(3).toString();

        ui->tw_clients->insertRow(row);

        QTableWidgetItem *itemName = new QTableWidgetItem(name);

        itemName->setData(Qt::UserRole, clientId);

        itemName->setData(Qt::UserRole + 1, type);

        if (phone.trimmed().isEmpty()) {
            phone = "—";
        }
        QTableWidgetItem *itemPhone = new QTableWidgetItem(phone);
        itemPhone->setTextAlignment(Qt::AlignCenter);

        ui->tw_clients->setItem(row, 0, itemName);
        ui->tw_clients->setItem(row, 1, itemPhone);

        row++;
    }

    ui->tw_clients->blockSignals(false);
}

void MainWidget::filterClientsTable(const QString &searchText)
{
    for (int row = 0; row < ui->tw_clients->rowCount(); ++row) {

        bool match = false;

        QTableWidgetItem *nameItem = ui->tw_clients->item(row, 0);
        QTableWidgetItem *phoneItem = ui->tw_clients->item(row, 1);

        if (nameItem && nameItem->text().contains(searchText, Qt::CaseInsensitive)) {
            match = true;
        }
        else if (phoneItem && phoneItem->text().contains(searchText, Qt::CaseInsensitive)) {
            match = true;
        }

        ui->tw_clients->setRowHidden(row, !match);
    }
}

void MainWidget::on_pb_nsi_edit_clicked()
{
    QTableWidgetItem *selectedRecord = ui->tw_nsi->item(ui->tw_nsi->currentRow(), 0);

    if (selectedRecord) {
        int itemId = selectedRecord->data(Qt::UserRole).toInt();
        openNsiEditor(itemId);
    }
}

void MainWidget::on_tw_nsi_itemSelectionChanged()
{
    bool hasSelection = (ui->tw_nsi->currentItem() != nullptr);

    ui->pb_nsi_edit->setEnabled(hasSelection);
    ui->pb_nsi_delete->setEnabled(hasSelection);
}

void MainWidget::on_pb_nsi_delete_clicked()
{
    QListWidgetItem *selectedDict = ui->lw_nsi_types->currentItem();
    if (!selectedDict) return;

    QString sysKey = selectedDict->data(Qt::UserRole).toString();

    QTableWidgetItem *selectedRecord = ui->tw_nsi->item(ui->tw_nsi->currentRow(), 0);
    if (!selectedRecord) return;

    int itemId = selectedRecord->data(Qt::UserRole).toInt();
    QString itemName = selectedRecord->text();

    QString tableName;
    QString tableHeader;

    if (sysKey == "sys_categories") {
        tableName = "categories";
        tableHeader = "Название категории";
    } else if (sysKey == "sys_units") {
        tableName = "units";
        tableHeader = "Единица измерения";
    } else if (sysKey == "sys_materials") {
        tableName = "materials";
        tableHeader = "Название материала";
    } else {
        QMessageBox::information(this, "Информация", "Удаление для данного раздела еще не реализовано.");
        return;
    }

    auto reply = QMessageBox::question(this, "Подтверждение",
                                       QString("Вы действительно хотите удалить запись «%1»?").arg(itemName),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    QString sql = QString("DELETE FROM %1 WHERE id = :id").arg(tableName);
    query.prepare(sql);
    query.bindValue(":id", itemId);

    if (query.exec()) {
        QMessageBox::information(this, "Успех", "Запись успешно удалена.");
        loadNsiTable(tableName, tableHeader);

    } else {
        QString errorMsg = query.lastError().text();

        if (errorMsg.contains("constraint", Qt::CaseInsensitive)) {
            QMessageBox::warning(this, "Удаление запрещено",
                                 "Невозможно удалить эту запись, так как она уже используется в других разделах системы.\n\n"
                                 "Сначала удалите или измените связанные данные.");
        } else {
            QMessageBox::critical(this, "Ошибка БД", "Не удалось удалить запись:\n" + errorMsg);
        }
    }
}

void MainWidget::on_pb_admin_user_delete_clicked()
{
    QListWidgetItem *selectedItem = ui->lw_admin_users->currentItem();
    if (!selectedItem) {
        QMessageBox::warning(this, "Внимание", "Выберите пользователя для удаления.");
        return;
    }

    int targetUserId = selectedItem->data(Qt::UserRole).toInt();
    QString userName = selectedItem->text();

    if (targetUserId == m_userId) {
        QMessageBox::critical(this, "Ошибка", "Вы не можете удалить собственную учетную запись, находясь в системе!");
        return;
    }

    if (targetUserId == 1) {
        QMessageBox::critical(this, "Ошибка", "Базового администратора системы удалить нельзя!");
        return;
    }

    auto reply = QMessageBox::question(this, "Подтверждение",
                                       QString("Вы действительно хотите безвозвратно удалить пользователя «%1»?").arg(userName),
                                       QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::No) return;

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось начать транзакцию базы данных.");
        return;
    }

    QSqlQuery query(db);

    query.prepare("DELETE FROM auth WHERE id = :id");
    query.bindValue(":id", targetUserId);
    if (!query.exec()) {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось удалить данные авторизации:\n" + query.lastError().text());
        return;
    }

    query.prepare("DELETE FROM users WHERE id = :id");
    query.bindValue(":id", targetUserId);
    if (!query.exec()) {
        db.rollback();

        if (query.lastError().text().contains("constraint", Qt::CaseInsensitive)) {
            QMessageBox::warning(this, "Удаление запрещено",
                                 "Невозможно удалить этого пользователя, так как он уже участвует в процессах системы (например, является автором проектов).\n\n"
                                 "Рекомендуется не удалять, а заблокировать пользователя (снять все права).");
        } else {
            QMessageBox::critical(this, "Ошибка БД", "Не удалось удалить пользователя:\n" + query.lastError().text());
        }
        return;
    }

    if (db.commit()) {
        QMessageBox::information(this, "Успех", "Пользователь успешно удален.");
        loadAdminUsers();
    } else {
        db.rollback();
        QMessageBox::critical(this, "Ошибка", "Не удалось зафиксировать изменения в базе данных.");
    }
}

void MainWidget::on_tw_clients_itemSelectionChanged()
{
    QTableWidgetItem *selectedItem = ui->tw_clients->item(ui->tw_clients->currentRow(), 0);
    bool hasSelection = (selectedItem != nullptr);

    if (!hasSelection) {
        ui->splitter_2->setSizes({1, 0});

        clearClientDetailsUI();
        return;
    }

    int clientId = selectedItem->data(Qt::UserRole).toInt();
    loadClientDetails(clientId);
    loadClientProjects(clientId);
    loadClientFinance(clientId);
    loadClientFiles(clientId);

    ui->splitter_2->setSizes({1, 1});
}

void MainWidget::clearClientDetailsUI()
{
    ui->lb_clients_clientName->clear();

    ui->le_clients_clientPhone->clear();
    ui->le_clients_clientEmail->clear();
    ui->le_clients_clientAddress->clear();
    ui->le_clients_clientPasport->clear();

    ui->pte_clients_clientNotes->clear();

    ui->tw_client_projects->clearContents();
    ui->tw_client_projects->setRowCount(0);
}

void MainWidget::loadClientDetails(int clientId)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "База данных не открыта!";
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT name, phone, email, address, passport, notes "
                  "FROM clients WHERE id = :id");
    query.bindValue(":id", clientId);

    if (query.exec() && query.next()) {

        ui->lb_clients_clientName->setText(query.value("name").toString());
        ui->le_clients_clientPhone->setText(query.value("phone").toString());
        ui->le_clients_clientEmail->setText(query.value("email").toString());

        ui->le_clients_clientAddress->setText(query.value("address").toString());
        ui->le_clients_clientPasport->setText(query.value("passport").toString());

        ui->pte_clients_clientNotes->setPlainText(query.value("notes").toString());

    } else {
        qWarning() << "Ошибка при загрузке деталей клиента:" << query.lastError().text();

        clearClientDetailsUI();
    }
}

void MainWidget::loadClientProjects(int clientId)
{
    ui->tw_client_projects->blockSignals(true);

    ui->tw_client_projects->clearContents();
    ui->tw_client_projects->setRowCount(0);
    ui->tw_client_projects->setColumnCount(4);
    ui->tw_client_projects->setHorizontalHeaderLabels({"Проект", "Статус", "Завершение", "Бюджет"});

    ui->tw_client_projects->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_client_projects->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_client_projects->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_client_projects->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    ui->tw_client_projects->verticalHeader()->setVisible(false);
    ui->tw_client_projects->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tw_client_projects->setSelectionBehavior(QAbstractItemView::SelectRows);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare("SELECT id, name, status, end_date, total_cost "
                  "FROM projects WHERE client_id = :client_id ORDER BY end_date DESC");
    query.bindValue(":client_id", clientId);

    if (!query.exec()) {
        qWarning() << "Ошибка при загрузке проектов клиента:" << query.lastError().text();
        return;
    }

    QLocale russianLocale(QLocale::Russian, QLocale::Russia);
    int row = 0;

    while (query.next()) {
        ui->tw_client_projects->insertRow(row);

        int projectId = query.value("id").toInt();
        QString name = query.value("name").toString();
        QString statusDb = query.value("status").toString();
        QDate endDate = query.value("end_date").toDate();
        double cost = query.value("total_cost").toDouble();

        QTableWidgetItem *itemName = new QTableWidgetItem(name);
        itemName->setData(Qt::UserRole, projectId);
        ui->tw_client_projects->setItem(row, 0, itemName);

        QString statusText;
        QColor statusColor;

        if (statusDb == "design") {
            statusText = "Проектирование";
            statusColor = QColor(255, 140, 0);
        } else if (statusDb == "building") {
            statusText = "Строительство";
            statusColor = QColor(0, 120, 215);
        } else if (statusDb == "finished") {
            statusText = "Завершено";
            statusColor = QColor(56, 142, 60);
        } else if (statusDb == "frozen") {
            statusText = "Заморожено";
            statusColor = QColor(120, 120, 120);
        }

        QTableWidgetItem *itemStatus = new QTableWidgetItem(statusText);
        itemStatus->setForeground(statusColor);
        itemStatus->setFont(QFont("Segoe UI", -1, QFont::Bold));
        itemStatus->setTextAlignment(Qt::AlignCenter);
        ui->tw_client_projects->setItem(row, 1, itemStatus);

        QString dateStr = endDate.isValid() ? endDate.toString("dd.MM.yyyy") : "—";
        QTableWidgetItem *itemDate = new QTableWidgetItem(dateStr);
        itemDate->setTextAlignment(Qt::AlignCenter);
        ui->tw_client_projects->setItem(row, 2, itemDate);

        QString costStr = russianLocale.toString(cost, 'f', 0) + " ₽";
        QTableWidgetItem *itemCost = new QTableWidgetItem(costStr);
        itemCost->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tw_client_projects->setItem(row, 3, itemCost);

        row++;
    }

    ui->tw_client_projects->blockSignals(false);
}

void MainWidget::loadClientFinance(int clientId)
{
    ui->tw_payments->blockSignals(true);
    ui->tw_payments->setColumnCount(4);
    ui->tw_payments->setHorizontalHeaderLabels({"Дата", "Сумма", "Назначение", "Статус"});

    ui->tw_payments->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_payments->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_payments->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tw_payments->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    ui->tw_payments->setRowCount(0);
    ui->tw_payments->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_payments->verticalHeader()->setVisible(false);
    ui->tw_payments->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return;

    QString totalsSql = R"(
        SELECT
            (SELECT COALESCE(SUM(total_cost), 0) FROM projects WHERE client_id = :id1) AS total_contract,
            (SELECT COALESCE(SUM(cp.amount), 0) FROM client_payments cp
             JOIN projects p ON cp.project_id = p.id WHERE p.client_id = :id2) AS total_paid
    )";

    QSqlQuery totalsQuery(db);
    totalsQuery.prepare(totalsSql);
    totalsQuery.bindValue(":id1", clientId);
    totalsQuery.bindValue(":id2", clientId);

    double totalContract = 0.0;
    double totalPaid = 0.0;

    if (totalsQuery.exec() && totalsQuery.next()) {
        totalContract = totalsQuery.value("total_contract").toDouble();
        totalPaid = totalsQuery.value("total_paid").toDouble();
    } else {
        qWarning() << "Ошибка при загрузке фин. итогов:" << totalsQuery.lastError().text();
    }

    double debt = totalContract - totalPaid;

    QLocale russianLocale(QLocale::Russian, QLocale::Russia);

    ui->lb_total_contract_sum->setText(russianLocale.toString(totalContract, 'f', 0) + " ₽");
    ui->lb_paid_sum->setText(russianLocale.toString(totalPaid, 'f', 0) + " ₽");
    ui->lb_debt_sum->setText(russianLocale.toString(debt, 'f', 0) + " ₽");

    if (debt > 0) {
        ui->lb_debt_sum->setStyleSheet("color: #D32F2F; font-weight: bold; font-size: 14pt;");
    } else {
        ui->lb_debt_sum->setStyleSheet("color: #388E3C; font-weight: bold; font-size: 14pt;");
    }

    QString paymentsSql = R"(
        SELECT cp.id, cp.payment_date, cp.amount, cp.purpose
        FROM client_payments cp
        JOIN projects p ON cp.project_id = p.id
        WHERE p.client_id = :id
        ORDER BY cp.payment_date DESC
    )";

    QSqlQuery paymentsQuery(db);
    paymentsQuery.prepare(paymentsSql);
    paymentsQuery.bindValue(":id", clientId);

    if (paymentsQuery.exec()) {
        int row = 0;
        while (paymentsQuery.next()) {
            ui->tw_payments->insertRow(row);

            int paymentId = paymentsQuery.value("id").toInt();
            QDateTime date = paymentsQuery.value("payment_date").toDateTime();
            double amount = paymentsQuery.value("amount").toDouble();
            QString purpose = paymentsQuery.value("purpose").toString();

            QString dateStr = date.isValid() ? date.toString("dd.MM.yyyy HH:mm") : "—";
            QTableWidgetItem *dateItem = new QTableWidgetItem(dateStr);
            dateItem->setData(Qt::UserRole, paymentId);
            ui->tw_payments->setItem(row, 0, dateItem);

            QString amountStr = russianLocale.toString(amount, 'f', 2) + " ₽";
            QTableWidgetItem *amountItem = new QTableWidgetItem(amountStr);
            amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            amountItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
            ui->tw_payments->setItem(row, 1, amountItem);

            ui->tw_payments->setItem(row, 2, new QTableWidgetItem(purpose));

            QTableWidgetItem *statusItem = new QTableWidgetItem("Зачислен");
            statusItem->setForeground(QColor(56, 142, 60));
            statusItem->setTextAlignment(Qt::AlignCenter);
            ui->tw_payments->setItem(row, 3, statusItem);

            row++;
        }
    } else {
        qWarning() << "Ошибка при загрузке таблицы платежей:" << paymentsQuery.lastError().text();
    }

    ui->tw_payments->blockSignals(false);
}

void MainWidget::loadProjectStages(int projectId)
{
    ui->tw_project_stages->blockSignals(true);
    ui->tw_project_stages->clearContents();
    ui->tw_project_stages->setRowCount(0);

    ui->tw_project_stages->setColumnCount(5);
    ui->tw_project_stages->setHorizontalHeaderLabels({"Этап", "План", "Факт (начало)", "Статус", "Ответственный"});

    ui->tw_project_stages->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_project_stages->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_project_stages->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_project_stages->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tw_project_stages->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    ui->tw_project_stages->verticalHeader()->setVisible(false);
    ui->tw_project_stages->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_project_stages->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return;

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT ps.id, ps.stage_name, ps.date_start_plan, ps.date_end_plan,
               ps.date_start_fact, ps.status, u.login AS responsible_name
        FROM project_stages ps
        LEFT JOIN users u ON ps.responsible_id = u.id
        WHERE ps.project_id = :project_id
        ORDER BY ps.date_start_plan ASC
    )");
    query.bindValue(":project_id", projectId);

    if (!query.exec()) {
        qWarning() << "Ошибка загрузки этапов проекта:" << query.lastError().text();
        return;
    }

    int totalStages = 0;
    int completedStages = 0;
    int row = 0;

    while (query.next()) {
        ui->tw_project_stages->insertRow(row);
        totalStages++;

        int stageId = query.value("id").toInt();
        QString name = query.value("stage_name").toString();
        QDate startPlan = query.value("date_start_plan").toDate();
        QDate endPlan = query.value("date_end_plan").toDate();
        QVariant startFactVar = query.value("date_start_fact");
        QString statusDb = query.value("status").toString();
        QString respName = query.value("responsible_name").toString();

        if (respName.isEmpty()) respName = "—";

        if (statusDb == "done") {
            completedStages++;
        }

        QTableWidgetItem *nameItem = new QTableWidgetItem(name);
        nameItem->setData(Qt::UserRole, stageId);
        ui->tw_project_stages->setItem(row, 0, nameItem);

        QString planStr = QString("%1 - %2").arg(startPlan.toString("dd.MM.yyyy"), endPlan.toString("dd.MM.yyyy"));
        QTableWidgetItem *planItem = new QTableWidgetItem(planStr);
        planItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_project_stages->setItem(row, 1, planItem);

        QString factStr = "—";
        QTableWidgetItem *factItem = new QTableWidgetItem();

        if (!startFactVar.isNull() && startFactVar.toDate().isValid()) {
            QDate startFact = startFactVar.toDate();
            factStr = startFact.toString("dd.MM.yyyy");

            if (startFact > startPlan) {
                factItem->setForeground(QColor(211, 47, 47));
                factItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
                factItem->setToolTip("Внимание: Этап начат с опозданием от плана!");
            }
        }
        factItem->setText(factStr);
        factItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_project_stages->setItem(row, 2, factItem);

        QString statusStr;
        QColor statusColor;

        if (statusDb == "pending") {
            statusStr = "Ожидает";
            statusColor = QColor(158, 158, 158);
        } else if (statusDb == "in_progress") {
            statusStr = "В работе";
            statusColor = QColor(25, 118, 210);
        } else if (statusDb == "done") {
            statusStr = "Завершен";
            statusColor = QColor(56, 142, 60);
        }

        QTableWidgetItem *statusItem = new QTableWidgetItem(statusStr);
        statusItem->setForeground(statusColor);
        statusItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
        statusItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_project_stages->setItem(row, 3, statusItem);

        QTableWidgetItem *respItem = new QTableWidgetItem(respName);
        respItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_project_stages->setItem(row, 4, respItem);

        row++;
    }

    int progressPercent = 0;
    if (totalStages > 0) {
        progressPercent = (completedStages * 100) / totalStages;
    }

    ui->pb_stage_progress->setValue(progressPercent);

    ui->tw_project_stages->blockSignals(false);
}

void MainWidget::loadProjectEstimates(int projectId)
{
    ui->tw_project_estimates->blockSignals(true);
    ui->tw_project_estimates->clearContents();
    ui->tw_project_estimates->setRowCount(0);

    ui->tw_project_estimates->setColumnCount(5);
    ui->tw_project_estimates->setHorizontalHeaderLabels({"Наименование", "Тип", "Кол-во", "Цена за ед.", "Итого"});

    ui->tw_project_estimates->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_project_estimates->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_project_estimates->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_project_estimates->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tw_project_estimates->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    ui->tw_project_estimates->verticalHeader()->setVisible(false);
    ui->tw_project_estimates->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_project_estimates->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        ui->tw_project_estimates->blockSignals(false);
        return;
    }

    QSqlQuery query(db);
    query.prepare(R"(
        SELECT pe.id, m.name AS material_name, pe.group_type,
               pe.quantity, pe.price_per_unit,
               (pe.quantity * pe.price_per_unit) AS total_row_sum
        FROM project_estimates pe
        LEFT JOIN materials m ON pe.material_id = m.id
        WHERE pe.project_id = :project_id
        ORDER BY pe.group_type, m.name ASC
    )");
    query.bindValue(":project_id", projectId);

    if (!query.exec()) {
        qWarning() << "Ошибка загрузки сметы:" << query.lastError().text();
        ui->tw_project_estimates->blockSignals(false);
        return;
    }

    QLocale russianLocale(QLocale::Russian, QLocale::Russia);
    double grandTotal = 0.0;
    int row = 0;

    while (query.next()) {
        ui->tw_project_estimates->insertRow(row);

        int estimateId = query.value("id").toInt();
        QString name = query.value("material_name").toString();
        QString groupTypeDb = query.value("group_type").toString();

        double quantity = query.value("quantity").toDouble();
        double price = query.value("price_per_unit").toDouble();
        double rowTotal = query.value("total_row_sum").toDouble();

        if (name.isEmpty()) name = "— Неизвестная позиция —";

        grandTotal += rowTotal;

        QTableWidgetItem *nameItem = new QTableWidgetItem(name);
        nameItem->setData(Qt::UserRole, estimateId);
        ui->tw_project_estimates->setItem(row, 0, nameItem);

        QString typeStr;
        QColor typeColor;

        if (groupTypeDb == "material") {
            typeStr = "Материал";
            typeColor = QColor(25, 118, 210);
        } else if (groupTypeDb == "work") {
            typeStr = "Работа";
            typeColor = QColor(230, 81, 0);
        } else if (groupTypeDb == "service") {
            typeStr = "Услуга";
            typeColor = QColor(156, 39, 176);
        } else {
            typeStr = groupTypeDb;
            typeColor = QColor(0, 0, 0);
        }

        QTableWidgetItem *typeItem = new QTableWidgetItem(typeStr);
        typeItem->setForeground(typeColor);
        typeItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
        typeItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_project_estimates->setItem(row, 1, typeItem);

        QTableWidgetItem *qtyItem = new QTableWidgetItem(russianLocale.toString(quantity, 'f', 3));
        qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tw_project_estimates->setItem(row, 2, qtyItem);

        QTableWidgetItem *priceItem = new QTableWidgetItem(russianLocale.toString(price, 'f', 2) + " ₽");
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tw_project_estimates->setItem(row, 3, priceItem);

        QTableWidgetItem *totalItem = new QTableWidgetItem(russianLocale.toString(rowTotal, 'f', 2) + " ₽");
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        totalItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
        ui->tw_project_estimates->setItem(row, 4, totalItem);

        row++;
    }

    if (ui->lb_estimate_total_sum) {
        ui->lb_estimate_total_sum->setText(russianLocale.toString(grandTotal, 'f', 2) + " ₽");
    }

    ui->tw_project_estimates->blockSignals(false);
}

void MainWidget::loadProjectFiles(int projectId)
{
    ui->tw_project_files->blockSignals(true);
    ui->tw_project_files->clearContents();
    ui->tw_project_files->setRowCount(0);

    ui->tw_project_files->setColumnCount(3);
    ui->tw_project_files->setHorizontalHeaderLabels({"Имя файла", "Тип", "Дата загрузки"});

    ui->tw_project_files->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_project_files->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_project_files->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    ui->tw_project_files->verticalHeader()->setVisible(false);
    ui->tw_project_files->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_project_files->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        ui->tw_project_files->blockSignals(false);
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id, file_name, file_path, file_type, uploaded_at "
                  "FROM project_files "
                  "WHERE project_id = :project_id "
                  "ORDER BY uploaded_at DESC");
    query.bindValue(":project_id", projectId);

    if (!query.exec()) {
        qWarning() << "Ошибка загрузки файлов проекта:" << query.lastError().text();
        ui->tw_project_files->blockSignals(false);
        return;
    }

    int row = 0;
    while (query.next()) {
        ui->tw_project_files->insertRow(row);

        int fileId = query.value("id").toInt();
        QString name = query.value("file_name").toString();
        QString path = query.value("file_path").toString();
        QString type = query.value("file_type").toString();
        QDateTime uploadedAt = query.value("uploaded_at").toDateTime();

        QTableWidgetItem *nameItem = new QTableWidgetItem(name);
        nameItem->setData(Qt::UserRole, fileId);
        nameItem->setData(Qt::UserRole + 1, path);

        QTableWidgetItem *typeItem = new QTableWidgetItem(type.toUpper());
        typeItem->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *dateItem = new QTableWidgetItem(uploadedAt.toString("dd.MM.yyyy HH:mm"));
        dateItem->setTextAlignment(Qt::AlignCenter);

        ui->tw_project_files->setItem(row, 0, nameItem);
        ui->tw_project_files->setItem(row, 1, typeItem);
        ui->tw_project_files->setItem(row, 2, dateItem);

        row++;
    }

    ui->tw_project_files->blockSignals(false);
}

void MainWidget::loadClientFiles(int clientId)
{
    ui->tw_client_files->blockSignals(true);
    ui->tw_client_files->clearContents();
    ui->tw_client_files->setRowCount(0);

    ui->tw_client_files->setColumnCount(3);
    ui->tw_client_files->setHorizontalHeaderLabels({"Имя файла", "Тип", "Дата загрузки"});

    ui->tw_client_files->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_client_files->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_client_files->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    ui->tw_client_files->verticalHeader()->setVisible(false);
    ui->tw_client_files->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_client_files->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        ui->tw_client_files->blockSignals(false);
        return;
    }

    QSqlQuery query(db);
    query.prepare("SELECT id, file_name, file_path, file_type, uploaded_at "
                  "FROM client_files "
                  "WHERE client_id = :client_id "
                  "ORDER BY uploaded_at DESC");
    query.bindValue(":client_id", clientId);

    if (!query.exec()) {
        ui->tw_client_files->blockSignals(false);
        return;
    }

    int row = 0;
    while (query.next()) {
        ui->tw_client_files->insertRow(row);

        int fileId = query.value("id").toInt();
        QString name = query.value("file_name").toString();
        QString path = query.value("file_path").toString();
        QString type = query.value("file_type").toString();
        QDateTime uploadedAt = query.value("uploaded_at").toDateTime();

        QTableWidgetItem *nameItem = new QTableWidgetItem(name);
        nameItem->setData(Qt::UserRole, fileId);
        nameItem->setData(Qt::UserRole + 1, path);

        QTableWidgetItem *typeItem = new QTableWidgetItem(type.toUpper());
        typeItem->setTextAlignment(Qt::AlignCenter);

        QTableWidgetItem *dateItem = new QTableWidgetItem(uploadedAt.toString("dd.MM.yyyy HH:mm"));
        dateItem->setTextAlignment(Qt::AlignCenter);

        ui->tw_client_files->setItem(row, 0, nameItem);
        ui->tw_client_files->setItem(row, 1, typeItem);
        ui->tw_client_files->setItem(row, 2, dateItem);

        row++;
    }

    ui->tw_client_files->blockSignals(false);
}

void MainWidget::loadHomeDashboard()
{
    QString baseStyle = "border-radius: 10px; padding: 15px;";
    ui->lb_stat_projects->setStyleSheet(baseStyle + "background-color: #e3f2fd; color: #1565c0; border: 1px solid #bbdefb;");
    ui->lb_stat_clients->setStyleSheet(baseStyle + "background-color: #e8f5e9; color: #2e7d32; border: 1px solid #c8e6c9;");
    ui->lb_stat_finance->setStyleSheet(baseStyle + "background-color: #f3e5f5; color: #7b1fa2; border: 1px solid #e1bee7;");

    ui->lb_new_client_card->setStyleSheet(baseStyle + "background-color: #fff3e0; color: #e65100; border: 1px solid #ffe0b2;");
    ui->lb_new_project_card->setStyleSheet(baseStyle + "background-color: #f1f8e9; color: #33691e; border: 1px solid #dcedc8;");

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) return;

    QSqlQuery query(db);

    int projectsCount = 0;
    if (query.exec("SELECT COUNT(*) FROM projects WHERE status IN ('design', 'building')") && query.next()) {
        projectsCount = query.value(0).toInt();
    }
    ui->lb_stat_projects->setText(QString("<html><center><span style='font-size: 11pt; font-weight: normal; color: #555555;'>Активные проекты</span><br/><span style='font-size: 22pt; font-weight: bold;'>%1</span></center></html>").arg(projectsCount));

    int clientsCount = 0;
    if (query.exec("SELECT COUNT(*) FROM clients") && query.next()) {
        clientsCount = query.value(0).toInt();
    }
    ui->lb_stat_clients->setText(QString("<html><center><span style='font-size: 11pt; font-weight: normal; color: #555555;'>Всего клиентов</span><br/><span style='font-size: 22pt; font-weight: bold;'>%1</span></center></html>").arg(clientsCount));

    double totalFinance = 0.0;
    if (query.exec("SELECT SUM(amount) FROM client_payments") && query.next()) {
        totalFinance = query.value(0).toDouble();
    }
    QLocale russianLocale(QLocale::Russian, QLocale::Russia);
    QString financeStr = russianLocale.toString(totalFinance, 'f', 2) + " ₽";
    ui->lb_stat_finance->setText(QString("<html><center><span style='font-size: 11pt; font-weight: normal; color: #555555;'>Общие поступления</span><br/><span style='font-size: 22pt; font-weight: bold;'>%1</span></center></html>").arg(financeStr));

    ui->lb_new_client_card->setText("<html><center><span style='font-size: 24pt; font-weight: bold;'>+</span><br/><span style='font-size: 11pt; font-weight: normal;'>Новый клиент</span></center></html>");
    ui->lb_new_project_card->setText("<html><center><span style='font-size: 24pt; font-weight: bold;'>+</span><br/><span style='font-size: 11pt; font-weight: normal;'>Новый проект</span></center></html>");

    ui->tw_home_deadlines->blockSignals(true);
    ui->tw_home_deadlines->clearContents();
    ui->tw_home_deadlines->setRowCount(0);

    ui->tw_home_deadlines->setColumnCount(3);
    ui->tw_home_deadlines->setHorizontalHeaderLabels({"Проект", "Этап", "Дедлайн"});
    ui->tw_home_deadlines->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_home_deadlines->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tw_home_deadlines->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_home_deadlines->verticalHeader()->setVisible(false);
    ui->tw_home_deadlines->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_home_deadlines->setEditTriggers(QAbstractItemView::NoEditTriggers);

    if (query.exec("SELECT p.id AS project_id, p.name AS project_name, ps.stage_name, ps.date_end_plan "
                   "FROM project_stages ps "
                   "JOIN projects p ON ps.project_id = p.id "
                   "WHERE ps.status IN ('pending', 'in_progress') AND ps.date_end_plan IS NOT NULL "
                   "ORDER BY ps.date_end_plan ASC LIMIT 15")) {
        int row = 0;
        while (query.next()) {
            ui->tw_home_deadlines->insertRow(row);

            QTableWidgetItem *projItem = new QTableWidgetItem(query.value("project_name").toString());
            projItem->setData(Qt::UserRole, query.value("project_id").toInt());

            QTableWidgetItem *stageItem = new QTableWidgetItem(query.value("stage_name").toString());

            QDate deadline = query.value("date_end_plan").toDate();
            QTableWidgetItem *dateItem = new QTableWidgetItem(deadline.toString("dd.MM.yyyy"));
            dateItem->setTextAlignment(Qt::AlignCenter);

            if (deadline < QDate::currentDate()) {
                dateItem->setForeground(QColor(211, 47, 47));
                dateItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
            } else if (deadline <= QDate::currentDate().addDays(7)) {
                dateItem->setForeground(QColor(230, 81, 0));
            }

            ui->tw_home_deadlines->setItem(row, 0, projItem);
            ui->tw_home_deadlines->setItem(row, 1, stageItem);
            ui->tw_home_deadlines->setItem(row, 2, dateItem);

            row++;
        }
    }
    ui->tw_home_deadlines->blockSignals(false);
}

void MainWidget::loadExportData()
{
    ui->cb_export_project->blockSignals(true);
    ui->cb_export_project->clear();
    ui->cb_export_project->addItem("Выберите проект", 0);

    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        QSqlQuery query(db);
        if (query.exec("SELECT id, name FROM projects ORDER BY name ASC")) {
            while (query.next()) {
                ui->cb_export_project->addItem(query.value("name").toString(), query.value("id").toInt());
            }
        }
    }
    ui->cb_export_project->blockSignals(false);

    ui->cb_export_doc_type->blockSignals(true);
    ui->cb_export_doc_type->clear();
    ui->cb_export_doc_type->addItem("Смета проекта", "estimate");
    ui->cb_export_doc_type->addItem("Договор подряда", "contract");
    ui->cb_export_doc_type->blockSignals(false);

    ui->cb_export_format->blockSignals(true);
    ui->cb_export_format->clear();
    ui->cb_export_format->addItem("PDF документ (*.pdf)", "pdf");
    ui->cb_export_format->addItem("HTML веб-страница (*.html)", "html");
    ui->cb_export_format->addItem("CSV таблица для Excel (*.csv)", "csv");
    ui->cb_export_format->blockSignals(false);

    updateExportPreview();
}

void MainWidget::updateExportPreview()
{
    int projectId = ui->cb_export_project->currentData().toInt();
    QString docType = ui->cb_export_doc_type->currentData().toString();

    if (projectId == 0) {
        ui->tb_export_preview->setHtml("<html><body><h3 style='color: gray; text-align: center; margin-top: 50px;'>Выберите проект для предпросмотра</h3></body></html>");
        ui->pb_export_save->setEnabled(false);
        return;
    }

    ui->pb_export_save->setEnabled(true);

    QSettings settings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
    QString compName = settings.value("Company/Name", "ООО «Строительная Компания»").toString();
    QString compAddress = settings.value("Company/Address", "—").toString();
    QString compPhone = settings.value("Company/Phone", "—").toString();

    QString logoPath = settings.value("Company/LogoPath").toString();
    if (!logoPath.isEmpty() && QFileInfo(logoPath).isRelative()) {
        logoPath = QCoreApplication::applicationDirPath() + "/" + logoPath;
    }

    QString html = "<html><head><style>body { font-family: 'Segoe UI', sans-serif; }</style></head><body>";

    html += "<table width='100%' style='border-bottom: 2px solid #1565c0; padding-bottom: 15px; margin-bottom: 20px;'><tr>";

    if (!logoPath.isEmpty() && QFile::exists(logoPath)) {
        html += QString("<td width='120'><img src='file:///%1' width='100' height='100' style='object-fit: contain;'/></td>").arg(logoPath);
    }

    html += QString("<td>"
                    "<span style='font-size: 16pt; font-weight: bold; color: #1565c0;'>%1</span><br/>"
                    "<span style='font-size: 10pt; color: #555555;'>Юр. адрес: %2<br/>"
                    "Тел.: %3</span>"
                    "</td></tr></table>")
                .arg(compName, compAddress, compPhone);

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);

    QString projectName;
    QString clientName;
    QString clientAddress;
    QString clientPhone;

    query.prepare("SELECT p.name, c.name AS client_name, c.address, c.phone "
                  "FROM projects p "
                  "JOIN clients c ON p.client_id = c.id "
                  "WHERE p.id = :id");
    query.bindValue(":id", projectId);

    if (query.exec() && query.next()) {
        projectName = query.value("name").toString();
        clientName = query.value("client_name").toString();
        clientAddress = query.value("address").toString();
        clientPhone = query.value("phone").toString();
    }

    QLocale loc(QLocale::Russian, QLocale::Russia);

    if (docType == "estimate") {
        html += QString("<h2 style='text-align: center; color: #333;'>СМЕТА РАСХОДОВ</h2>");
        html += QString("<h3 style='text-align: center; color: #666;'>по объекту: %1</h3>").arg(projectName);
        html += QString("<p><b>Заказчик:</b> %1<br><b>Телефон:</b> %2</p>").arg(clientName, clientPhone);

        html += "<table border='1' cellspacing='0' cellpadding='6' width='100%' style='border-collapse: collapse; border: 1px solid #ddd;'>";
        html += "<tr bgcolor='#1565c0' style='color: white;'><th>Наименование</th><th>Кол-во</th><th>Цена (₽)</th><th>Сумма (₽)</th></tr>";

        query.prepare("SELECT m.name AS material_name, pe.quantity, pe.price_per_unit, (pe.quantity * pe.price_per_unit) AS total "
                      "FROM project_estimates pe "
                      "LEFT JOIN materials m ON pe.material_id = m.id "
                      "WHERE pe.project_id = :id "
                      "ORDER BY pe.group_type, m.name");
        query.bindValue(":id", projectId);

        double grandTotal = 0.0;
        if (query.exec()) {
            while (query.next()) {
                QString mName = query.value("material_name").toString();
                double qty = query.value("quantity").toDouble();
                double price = query.value("price_per_unit").toDouble();
                double total = query.value("total").toDouble();
                grandTotal += total;

                html += QString("<tr><td>%1</td><td align='center'>%2</td><td align='right'>%3</td><td align='right'>%4</td></tr>")
                            .arg(mName)
                            .arg(loc.toString(qty, 'f', 3))
                            .arg(loc.toString(price, 'f', 2))
                            .arg(loc.toString(total, 'f', 2));
            }
        }

        html += QString("<tr bgcolor='#f5f5f5'><td colspan='3' align='right'><b>ИТОГО ПО СМЕТЕ:</b></td><td align='right' style='color: #1565c0;'><b>%1</b></td></tr>")
                    .arg(loc.toString(grandTotal, 'f', 2));
        html += "</table>";
        html += "<br><br><table width='100%'><tr>";
        html += QString("<td><b>Подрядчик (%1):</b><br><br>___________________</td>").arg(compName);
        html += QString("<td><b>Заказчик (%1):</b><br><br>___________________</td>").arg(clientName);
        html += "</tr></table>";

    } else if (docType == "contract") {
        html += QString("<h2 style='text-align: center; color: #333;'>ДОГОВОР ПОДРЯДА № %1</h2>").arg(projectId);
        html += QString("<p style='text-align: right;'>Дата: %1</p>").arg(QDate::currentDate().toString("dd.MM.yyyy"));
        html += QString("<p><b>%1</b>, именуемое в дальнейшем «Подрядчик», в лице руководителя, с одной стороны, и <b>%2</b>, именуемый(ая) в дальнейшем «Заказчик», с другой стороны, заключили настоящий договор о нижеследующем:</p>").arg(compName, clientName);
        html += "<p><b>1. ПРЕДМЕТ ДОГОВОРА</b><br>1.1. Подрядчик обязуется выполнить строительные/проектные работы по объекту <b>«" + projectName + "»</b>, а Заказчик обязуется принять результат работ и оплатить их.</p>";
        html += "<p><b>2. ПРАВА И ОБЯЗАННОСТИ СТОРОН</b><br>2.1. Подрядчик обязан выполнить работы качественно и в срок.<br>2.2. Заказчик обязан обеспечить доступ на объект и своевременную оплату.</p>";

        html += "<br><br><table width='100%' style='border-top: 1px solid #ccc; padding-top: 15px;'><tr>";
        html += QString("<td width='50%' valign='top'><b>ПОДРЯДЧИК:</b><br/>%1<br/>Адрес: %2<br/>Тел: %3<br/><br/><br/>___________ / ___________</td>")
                    .arg(compName, compAddress, compPhone);
        html += QString("<td width='50%' valign='top'><b>ЗАКАЗЧИК:</b><br/>%1<br/>Адрес: %2<br/>Тел: %3<br/><br/><br/>___________ / ___________</td>")
                    .arg(clientName, clientAddress, clientPhone);
        html += "</tr></table>";
    }

    html += "</body></html>";
    ui->tb_export_preview->setHtml(html);
}

void MainWidget::loadCompanySettings()
{
    QString iniPath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(iniPath, QSettings::IniFormat);

    ui->le_company_name->setText(settings.value("Company/Name").toString());
    ui->le_legal_address->setText(settings.value("Company/Address").toString());
    ui->le_contact_phone->setText(settings.value("Company/Phone").toString());

    QString logoPath = settings.value("Company/LogoPath").toString();

    if (!logoPath.isEmpty() && QFileInfo(logoPath).isRelative()) {
        logoPath = QCoreApplication::applicationDirPath() + "/" + logoPath;
    }

    m_companyLogoPath = settings.value("Company/LogoPath").toString();

    if (!logoPath.isEmpty() && QFile::exists(logoPath)) {
        QPixmap pixmap(logoPath);
        ui->lb_logo->setPixmap(pixmap.scaled(ui->lb_logo->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->lb_logo->setText("Логотип не выбран");
    }
}

void MainWidget::saveCompanySettings()
{
    QString iniPath = QCoreApplication::applicationDirPath() + "/config.ini";
    QSettings settings(iniPath, QSettings::IniFormat);

    settings.setValue("Company/Name", ui->le_company_name->text().trimmed());
    settings.setValue("Company/Address", ui->le_legal_address->text().trimmed());
    settings.setValue("Company/Phone", ui->le_contact_phone->text().trimmed());
    settings.setValue("Company/LogoPath", m_companyLogoPath);

    settings.sync();

    QMessageBox::information(this, "Успех", "Настройки компании успешно сохранены!");
}

void MainWidget::loadProjectsTable()
{
    ui->tw_projects_list->blockSignals(true);

    ui->tw_projects_list->setColumnCount(6);
    ui->tw_projects_list->setHorizontalHeaderLabels({
        "Название проекта", "Заказчик", "Статус", "Начало", "План. завершение", "Бюджет"
    });

    ui->tw_projects_list->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_projects_list->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tw_projects_list->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_projects_list->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);

    ui->tw_projects_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_projects_list->verticalHeader()->setVisible(false);
    ui->tw_projects_list->setEditTriggers(QAbstractItemView::NoEditTriggers);

    ui->tw_projects_list->setRowCount(0);

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "База данных не открыта!";
        return;
    }

    QSqlQuery query(db);
    QString sql = R"(
        SELECT
            p.id,
            p.name,
            c.name AS client_name,
            p.status,
            p.start_date,
            p.end_date,
            p.total_cost
        FROM projects p
        LEFT JOIN clients c ON p.client_id = c.id
        ORDER BY p.start_date DESC
    )";

    if (!query.exec(sql)) {
        qCritical() << "Ошибка при загрузке проектов:" << query.lastError().text();
        return;
    }

    QLocale russianLocale(QLocale::Russian, QLocale::Russia);

    int row = 0;
    while (query.next()) {
        ui->tw_projects_list->insertRow(row);

        int projectId = query.value("id").toInt();
        QString projectName = query.value("name").toString();
        QString clientName = query.value("client_name").toString();
        if (clientName.isEmpty()) clientName = "—";

        QString statusDb = query.value("status").toString();
        QDate startDate = query.value("start_date").toDate();
        QDate endDate = query.value("end_date").toDate();
        double budget = query.value("total_cost").toDouble();

        QTableWidgetItem *itemName = new QTableWidgetItem(projectName);
        itemName->setData(Qt::UserRole, projectId);
        ui->tw_projects_list->setItem(row, 0, itemName);

        ui->tw_projects_list->setItem(row, 1, new QTableWidgetItem(clientName));

        QString statusText;
        QColor statusColor;

        if (statusDb == "design") {
            statusText = "Проектирование";
            statusColor = QColor(255, 140, 0);
        } else if (statusDb == "building") {
            statusText = "Строительство";
            statusColor = QColor(0, 120, 215);
        } else if (statusDb == "finished") {
            statusText = "Завершено";
            statusColor = QColor(56, 142, 60);
        } else if (statusDb == "frozen") {
            statusText = "Заморожено";
            statusColor = QColor(120, 120, 120);
        } else {
            statusText = "Неизвестно";
            statusColor = QColor(0, 0, 0);
        }

        QTableWidgetItem *itemStatus = new QTableWidgetItem(statusText);
        itemStatus->setForeground(statusColor);
        itemStatus->setFont(QFont("Segoe UI", -1, QFont::Bold));
        itemStatus->setTextAlignment(Qt::AlignCenter);
        ui->tw_projects_list->setItem(row, 2, itemStatus);

        QString startStr = startDate.isValid() ? startDate.toString("dd.MM.yyyy") : "—";
        QString endStr = endDate.isValid() ? endDate.toString("dd.MM.yyyy") : "—";

        ui->tw_projects_list->setItem(row, 3, new QTableWidgetItem(startStr));
        ui->tw_projects_list->setItem(row, 4, new QTableWidgetItem(endStr));

        QString budgetStr = russianLocale.toString(budget, 'f', 2) + " ₽";
        QTableWidgetItem *itemBudget = new QTableWidgetItem(budgetStr);
        itemBudget->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tw_projects_list->setItem(row, 5, itemBudget);

        row++;
    }

    ui->tw_projects_list->blockSignals(false);
}

void MainWidget::on_tw_projects_list_itemSelectionChanged()
{
    QTableWidgetItem *selectedItem = ui->tw_projects_list->item(ui->tw_projects_list->currentRow(), 0);
    bool hasSelection = (selectedItem != nullptr);

    if (!hasSelection) {
        ui->splitter_5->setSizes({1, 0});

        clearProjectDetailsUI();
        return;
    }

    int projectId = selectedItem->data(Qt::UserRole).toInt();

    loadProjectDetails(projectId);
    loadProjectStages(projectId);
    loadProjectEstimates(projectId);
    loadProjectFiles(projectId);

    ui->splitter_5->setSizes({1, 1});
}

void MainWidget::clearProjectDetailsUI()
{
    ui->lb_proj_name->clear();

    ui->lb_proj_client_ref->clear();

    ui->lb_proj_status->clear();

    ui->pb_stage_progress->setValue(0);

    ui->pte_project_description->clear();

    ui->lb_total_contract_sum->clear();
    ui->lb_paid_sum->clear();
    ui->lb_debt_sum->clear();

    ui->lb_debt_sum->setStyleSheet("");

    ui->tw_payments->clearContents();
    ui->tw_payments->setRowCount(0);

    ui->tw_project_stages->clearContents();
    ui->tw_project_stages->setRowCount(0);

    ui->pb_stage_progress->setValue(0);

    ui->tw_project_estimates->clearContents();
    ui->tw_project_estimates->setRowCount(0);

    if(ui->lb_estimate_total_sum) {
        ui->lb_estimate_total_sum->setText("0 ₽");
    }
}

void MainWidget::loadProjectDetails(int projectId)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qWarning() << "База данных не открыта!";
        return;
    }

    QSqlQuery query(db);
    QString sql = R"(
        SELECT
            p.name,
            p.client_id,
            c.name AS client_name,
            p.status,
            p.address
        FROM projects p
        LEFT JOIN clients c ON p.client_id = c.id
        WHERE p.id = :id
    )";

    query.prepare(sql);
    query.bindValue(":id", projectId);

    if (query.exec() && query.next()) {

        QString projectName = query.value("name").toString();
        int clientId = query.value("client_id").toInt();
        QString clientName = query.value("client_name").toString();
        QString statusDb = query.value("status").toString();
        QString address = query.value("address").toString();

        if (clientName.isEmpty()) clientName = "Не указан";

        ui->lb_proj_name->setText(projectName);

        ui->lb_proj_client_ref->setText(QString("<a href=\"%1\" style=\"color: #0078D7; text-decoration: none;\">%2</a>")
                                            .arg(clientId)
                                            .arg(clientName));

        ui->lb_proj_client_ref->setTextFormat(Qt::RichText);

        QString statusText;
        int progressValue = 0;

        if (statusDb == "design") {
            statusText = "Проектирование";
            progressValue = 25;
        } else if (statusDb == "building") {
            statusText = "Строительство";
            progressValue = 70;
        } else if (statusDb == "finished") {
            statusText = "Завершено";
            progressValue = 100;
        } else if (statusDb == "frozen") {
            statusText = "Заморожено";
            progressValue = 0;
        } else {
            statusText = "Неизвестно";
        }

        ui->lb_proj_status->setText(statusText);
        ui->pb_stage_progress->setValue(progressValue);

        ui->pte_project_description->setPlainText(QString("Адрес объекта:\n%1").arg(address));

    } else {
        qWarning() << "Ошибка при загрузке деталей проекта:" << query.lastError().text();
        clearProjectDetailsUI();
    }
}

void MainWidget::on_pb_client_edit_clicked()
{
    QTableWidgetItem *selectedItem = ui->tw_clients->item(ui->tw_clients->currentRow(), 0);
    if (!selectedItem) return;

    int clientId = selectedItem->data(Qt::UserRole).toInt();

    ClientEditorDialog dialog(clientId, this);

    if (dialog.exec() == QDialog::Accepted) {

        loadClientsTable();

        loadClientDetails(clientId);

        for (int row = 0; row < ui->tw_clients->rowCount(); ++row) {
            QTableWidgetItem *item = ui->tw_clients->item(row, 0);
            if (item && item->data(Qt::UserRole).toInt() == clientId) {
                ui->tw_clients->setCurrentItem(item);
                break;
            }
        }
    }
}

void MainWidget::on_pb_client_create_clicked()
{
    ClientEditorDialog dialog(-1, this);

    if (dialog.exec() == QDialog::Accepted) {

        loadClientsTable();

        ui->splitter_2->setSizes({1, 0});
        clearClientDetailsUI();

        QMessageBox::information(this, "Успех", "Новый клиент успешно добавлен в базу!");
    }
}

void MainWidget::on_tw_client_projects_itemDoubleClicked(QTableWidgetItem *item)
{
    if (!item) return;

    int row = item->row();
    QTableWidgetItem *firstColItem = ui->tw_client_projects->item(row, 0);

    if (!firstColItem) return;

    int projectId = firstColItem->data(Qt::UserRole).toInt();

    ui->lw_main->setCurrentRow(2);
    sw_main_change(2);

    for (int r = 0; r < ui->tw_projects_list->rowCount(); ++r) {
        QTableWidgetItem *projItem = ui->tw_projects_list->item(r, 0);

        if (projItem && projItem->data(Qt::UserRole).toInt() == projectId) {

            ui->tw_projects_list->setCurrentItem(projItem);

            ui->tw_projects_list->scrollToItem(projItem);

            break;
        }
    }
}

void MainWidget::on_pb_add_payment_clicked()
{
    QTableWidgetItem *selectedItem = ui->tw_clients->item(ui->tw_clients->currentRow(), 0);
    if (!selectedItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите клиента из списка.");
        return;
    }

    int clientId = selectedItem->data(Qt::UserRole).toInt();

    PaymentDialog dialog(clientId, this);

    if (dialog.exec() == QDialog::Accepted) {
        loadClientFinance(clientId);

        QMessageBox::information(this, "Успех", "Платеж успешно зачислен!");
    }
}

void MainWidget::on_pb_project_create_clicked()
{
    ProjectEditorDialog dialog(-1, this);

    if (dialog.exec() == QDialog::Accepted) {

        loadProjectsTable();

        ui->splitter_5->setSizes({1, 0});
        clearProjectDetailsUI();

        QMessageBox::information(this, "Успех", "Новый проект успешно добавлен в базу!");
    }
}

void MainWidget::on_pb_project_edit_clicked()
{
    QTableWidgetItem *selectedItem = ui->tw_projects_list->item(ui->tw_projects_list->currentRow(), 0);
    if (!selectedItem) {
        return;
    }

    int projectId = selectedItem->data(Qt::UserRole).toInt();

    ProjectEditorDialog dialog(projectId, this);

    if (dialog.exec() == QDialog::Accepted) {

        loadProjectsTable();

        loadProjectDetails(projectId);

        QMessageBox::information(this, "Успех", "Данные проекта обвнолены!");

        for (int row = 0; row < ui->tw_projects_list->rowCount(); ++row) {
            QTableWidgetItem *item = ui->tw_projects_list->item(row, 0);
            if (item && item->data(Qt::UserRole).toInt() == projectId) {
                ui->tw_projects_list->setCurrentItem(item);
                break;
            }
        }
    }
}

void MainWidget::on_pb_stage_add_clicked()
{
    QTableWidgetItem *projItem = ui->tw_projects_list->item(ui->tw_projects_list->currentRow(), 0);
    if (!projItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите проект в главном списке.");
        return;
    }

    int projectId = projItem->data(Qt::UserRole).toInt();

    ProjectStageDialog dialog(projectId, -1, this);

    if (dialog.exec() == QDialog::Accepted) {
        loadProjectStages(projectId);

        QMessageBox::information(this, "Успех", "Новый этап успешно добавлен!");
    }
}

void MainWidget::on_pb_stage_edit_clicked()
{
    QTableWidgetItem *projItem = ui->tw_projects_list->item(ui->tw_projects_list->currentRow(), 0);
    if (!projItem) return;
    int projectId = projItem->data(Qt::UserRole).toInt();

    QTableWidgetItem *stageItem = ui->tw_project_stages->item(ui->tw_project_stages->currentRow(), 0);
    if (!stageItem) {
        QMessageBox::warning(this, "Внимание", "Выберите этап для редактирования.");
        return;
    }

    int stageId = stageItem->data(Qt::UserRole).toInt();

    ProjectStageDialog dialog(projectId, stageId, this);

    if (dialog.exec() == QDialog::Accepted) {
        loadProjectStages(projectId);

        for (int row = 0; row < ui->tw_project_stages->rowCount(); ++row) {
            QTableWidgetItem *item = ui->tw_project_stages->item(row, 0);
            if (item && item->data(Qt::UserRole).toInt() == stageId) {
                ui->tw_project_stages->setCurrentItem(item);
                break;
            }
        }

        QMessageBox::information(this, "Успех", "Изменения этапа успешно сохранены!");
    }
}

void MainWidget::on_pb_stage_delete_clicked()
{
    QTableWidgetItem *stageItem = ui->tw_project_stages->item(ui->tw_project_stages->currentRow(), 0);
    if (!stageItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите этап для удаления.");
        return;
    }

    int stageId = stageItem->data(Qt::UserRole).toInt();

    QTableWidgetItem *projItem = ui->tw_projects_list->item(ui->tw_projects_list->currentRow(), 0);
    if (!projItem) return;
    int projectId = projItem->data(Qt::UserRole).toInt();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение удаления",
                                  "Вы действительно хотите удалить выбранный этап?\n"
                                  "Это действие нельзя отменить, и оно повлияет на расчет прогресса проекта.",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM project_stages WHERE id = :id");
    query.bindValue(":id", stageId);

    if (query.exec()) {
        loadProjectStages(projectId);

        QMessageBox::information(this, "Успех", "Этап успешно удален!");
    } else {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось удалить этап:\n" + query.lastError().text());
    }
}

void MainWidget::on_tw_project_estimates_itemSelectionChanged()
{
    bool hasSelection = ui->tw_project_estimates->currentRow() >= 0;
    ui->pb_estimate_edit->setEnabled(hasSelection);
    ui->pb_estimate_delete->setEnabled(hasSelection);
}


void MainWidget::on_pb_estimate_add_clicked()
{
    QTableWidgetItem *projItem = ui->tw_projects_list->item(ui->tw_projects_list->currentRow(), 0);
    if (!projItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите проект в главном списке.");
        return;
    }

    int projectId = projItem->data(Qt::UserRole).toInt();

    ProjectEstimateDialog dialog(projectId, -1, this);

    if (dialog.exec() == QDialog::Accepted) {
        loadProjectEstimates(projectId);
        QMessageBox::information(this, "Успех", "Позиция успешно добавлена в смету!");
    }
}


void MainWidget::on_pb_estimate_edit_clicked()
{
    QTableWidgetItem *projItem = ui->tw_projects_list->item(ui->tw_projects_list->currentRow(), 0);
    if (!projItem) return;
    int projectId = projItem->data(Qt::UserRole).toInt();

    QTableWidgetItem *estItem = ui->tw_project_estimates->item(ui->tw_project_estimates->currentRow(), 0);
    if (!estItem) {
        QMessageBox::warning(this, "Внимание", "Выберите позицию сметы для редактирования.");
        return;
    }

    int estimateId = estItem->data(Qt::UserRole).toInt();

    ProjectEstimateDialog dialog(projectId, estimateId, this);

    if (dialog.exec() == QDialog::Accepted) {
        loadProjectEstimates(projectId);

        for (int row = 0; row < ui->tw_project_estimates->rowCount(); ++row) {
            QTableWidgetItem *item = ui->tw_project_estimates->item(row, 0);
            if (item && item->data(Qt::UserRole).toInt() == estimateId) {
                ui->tw_project_estimates->setCurrentItem(item);
                break;
            }
        }

        QMessageBox::information(this, "Успех", "Изменения позиции успешно сохранены!");
    }
}


void MainWidget::on_pb_estimate_delete_clicked()
{
    QTableWidgetItem *estItem = ui->tw_project_estimates->item(ui->tw_project_estimates->currentRow(), 0);
    if (!estItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите позицию сметы для удаления.");
        return;
    }

    int estimateId = estItem->data(Qt::UserRole).toInt();

    QTableWidgetItem *projItem = ui->tw_projects_list->item(ui->tw_projects_list->currentRow(), 0);
    if (!projItem) return;
    int projectId = projItem->data(Qt::UserRole).toInt();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение удаления",
                                  "Вы действительно хотите удалить выбранную позицию из сметы?",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM project_estimates WHERE id = :id");
    query.bindValue(":id", estimateId);

    if (query.exec()) {
        loadProjectEstimates(projectId);
        QMessageBox::information(this, "Успех", "Позиция успешно удалена из сметы!");
    } else {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось удалить позицию:\n" + query.lastError().text());
    }
}

void MainWidget::on_pb_client_project_add_clicked()
{
    QTableWidgetItem *clientItem = ui->tw_clients->item(ui->tw_clients->currentRow(), 0);
    if (!clientItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите клиента в списке.");
        return;
    }

    int clientId = clientItem->data(Qt::UserRole).toInt();

    ProjectEditorDialog dialog(-1, this);
    dialog.setClient(clientId);

    if (dialog.exec() == QDialog::Accepted) {
        loadProjectsTable();

        loadClientProjects(clientId);

        QMessageBox::information(this, "Успех", "Проект для клиента успешно создан!");
    }
}

void MainWidget::on_pb_project_file_add_clicked()
{
    QTableWidgetItem *projItem = ui->tw_projects_list->item(ui->tw_projects_list->currentRow(), 0);
    if (!projItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите проект в главном списке.");
        return;
    }

    int projectId = projItem->data(Qt::UserRole).toInt();

    QString sourceFilePath = QFileDialog::getOpenFileName(this, "Выберите файл для загрузки", "", "Все файлы (*.*)");
    if (sourceFilePath.isEmpty()) {
        return;
    }

    QString destFolder = FileStorageManager::getProjectFolder(projectId);
    QString savedFilePath = FileStorageManager::copyFileToStorage(sourceFilePath, destFolder);

    if (savedFilePath.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось скопировать файл в хранилище.");
        return;
    }

    QFileInfo fileInfo(savedFilePath);
    QString fileName = fileInfo.fileName();
    QString fileType = fileInfo.suffix();

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("INSERT INTO project_files (project_id, file_name, file_path, file_type, uploaded_at) "
                  "VALUES (:project_id, :file_name, :file_path, :file_type, :uploaded_at)");
    query.bindValue(":project_id", projectId);
    query.bindValue(":file_name", fileName);
    query.bindValue(":file_path", savedFilePath);
    query.bindValue(":file_type", fileType);
    query.bindValue(":uploaded_at", QDateTime::currentDateTime());

    if (query.exec()) {
        loadProjectFiles(projectId);
        QMessageBox::information(this, "Успех", "Файл успешно загружен!");
    } else {
        QFile::remove(savedFilePath);
        QMessageBox::critical(this, "Ошибка БД", "Не удалось сохранить запись о файле:\n" + query.lastError().text());
    }
}

void MainWidget::on_tw_project_files_itemDoubleClicked(QTableWidgetItem *item)
{
    int row = item->row();
    QTableWidgetItem *nameItem = ui->tw_project_files->item(row, 0);

    if (nameItem) {
        QString filePath = nameItem->data(Qt::UserRole + 1).toString();
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    }
}

void MainWidget::on_pb_project_file_delete_clicked()
{
    QTableWidgetItem *fileItem = ui->tw_project_files->item(ui->tw_project_files->currentRow(), 0);
    if (!fileItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите файл для удаления.");
        return;
    }

    QTableWidgetItem *projItem = ui->tw_projects_list->item(ui->tw_projects_list->currentRow(), 0);
    if (!projItem) return;
    int projectId = projItem->data(Qt::UserRole).toInt();

    int fileId = fileItem->data(Qt::UserRole).toInt();
    QString filePath = fileItem->data(Qt::UserRole + 1).toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение удаления",
                                  "Вы действительно хотите удалить этот файл?\nОН БУДЕТ УДАЛЕН С ЖЕСТКОГО ДИСКА!",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM project_files WHERE id = :id");
    query.bindValue(":id", fileId);

    if (query.exec()) {
        QFile::remove(filePath);

        loadProjectFiles(projectId);
        QMessageBox::information(this, "Успех", "Файл успешно удален!");
    } else {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось удалить запись о файле:\n" + query.lastError().text());
    }
}

void MainWidget::on_pb_project_open_folder_clicked()
{
    QTableWidgetItem *projItem = ui->tw_projects_list->item(ui->tw_projects_list->currentRow(), 0);
    if (!projItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите проект в главном списке.");
        return;
    }

    int projectId = projItem->data(Qt::UserRole).toInt();
    QString folderPath = FileStorageManager::getProjectFolder(projectId);

    QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}

void MainWidget::on_pb_client_file_add_clicked()
{
    QTableWidgetItem *clientItem = ui->tw_clients->item(ui->tw_clients->currentRow(), 0);
    if (!clientItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите клиента в списке.");
        return;
    }

    int clientId = clientItem->data(Qt::UserRole).toInt();

    QString sourceFilePath = QFileDialog::getOpenFileName(this, "Выберите файл для загрузки", "", "Все файлы (*.*)");
    if (sourceFilePath.isEmpty()) {
        return;
    }

    QString destFolder = FileStorageManager::getClientFolder(clientId);
    QString savedFilePath = FileStorageManager::copyFileToStorage(sourceFilePath, destFolder);

    if (savedFilePath.isEmpty()) {
        QMessageBox::critical(this, "Ошибка", "Не удалось скопировать файл в хранилище.");
        return;
    }

    QFileInfo fileInfo(savedFilePath);
    QString fileName = fileInfo.fileName();
    QString fileType = fileInfo.suffix();

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("INSERT INTO client_files (client_id, file_name, file_path, file_type, uploaded_at) "
                  "VALUES (:client_id, :file_name, :file_path, :file_type, :uploaded_at)");
    query.bindValue(":client_id", clientId);
    query.bindValue(":file_name", fileName);
    query.bindValue(":file_path", savedFilePath);
    query.bindValue(":file_type", fileType);
    query.bindValue(":uploaded_at", QDateTime::currentDateTime());

    if (query.exec()) {
        loadClientFiles(clientId);
        QMessageBox::information(this, "Успех", "Файл успешно загружен!");
    } else {
        QFile::remove(savedFilePath);
        QMessageBox::critical(this, "Ошибка БД", "Не удалось сохранить запись о файле:\n" + query.lastError().text());
    }
}

void MainWidget::on_pb_client_file_delete_clicked()
{
    QTableWidgetItem *fileItem = ui->tw_client_files->item(ui->tw_client_files->currentRow(), 0);
    if (!fileItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите файл для удаления.");
        return;
    }

    QTableWidgetItem *clientItem = ui->tw_clients->item(ui->tw_clients->currentRow(), 0);
    if (!clientItem) return;
    int clientId = clientItem->data(Qt::UserRole).toInt();

    int fileId = fileItem->data(Qt::UserRole).toInt();
    QString filePath = fileItem->data(Qt::UserRole + 1).toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение удаления",
                                  "Вы действительно хотите удалить этот файл?\nОН БУДЕТ УДАЛЕН С ЖЕСТКОГО ДИСКА!",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::No) {
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    QSqlQuery query(db);
    query.prepare("DELETE FROM client_files WHERE id = :id");
    query.bindValue(":id", fileId);

    if (query.exec()) {
        QFile::remove(filePath);
        loadClientFiles(clientId);
        QMessageBox::information(this, "Успех", "Файл успешно удален!");
    } else {
        QMessageBox::critical(this, "Ошибка БД", "Не удалось удалить запись о файле:\n" + query.lastError().text());
    }
}

void MainWidget::on_tw_client_files_itemDoubleClicked(QTableWidgetItem *item)
{
    int row = item->row();
    QTableWidgetItem *nameItem = ui->tw_client_files->item(row, 0);

    if (nameItem) {
        QString filePath = nameItem->data(Qt::UserRole + 1).toString();
        QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
    }
}

void MainWidget::on_pb_client_open_folder_clicked()
{
    QTableWidgetItem *clientItem = ui->tw_clients->item(ui->tw_clients->currentRow(), 0);
    if (!clientItem) {
        QMessageBox::warning(this, "Внимание", "Сначала выберите клиента в списке.");
        return;
    }

    int clientId = clientItem->data(Qt::UserRole).toInt();
    QString folderPath = FileStorageManager::getClientFolder(clientId);

    QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}

void MainWidget::on_lb_stat_projects_clicked()
{
    ui->lw_main->setCurrentRow(2);
}

void MainWidget::on_lb_stat_clients_clicked()
{
    ui->lw_main->setCurrentRow(1);
}

void MainWidget::on_tw_home_deadlines_itemDoubleClicked(QTableWidgetItem *item)
{
    int row = item->row();
    QTableWidgetItem *projItem = ui->tw_home_deadlines->item(row, 0);

    if (!projItem) {
        return;
    }

    int projectId = projItem->data(Qt::UserRole).toInt();

    ui->lw_main->setCurrentRow(2);

    for (int i = 0; i < ui->tw_projects_list->rowCount(); ++i) {
        QTableWidgetItem *listItem = ui->tw_projects_list->item(i, 0);
        if (listItem && listItem->data(Qt::UserRole).toInt() == projectId) {
            ui->tw_projects_list->setCurrentItem(listItem);
            break;
        }
    }
}

void MainWidget::on_lb_new_client_card_clicked()
{
    ClientEditorDialog dialog(-1, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadHomeDashboard();
    }
}

void MainWidget::on_lb_new_project_card_clicked()
{
    ProjectEditorDialog dialog(-1, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadHomeDashboard();
    }
}

void MainWidget::on_cb_export_project_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    updateExportPreview();
}

void MainWidget::on_cb_export_doc_type_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    updateExportPreview();
}

void MainWidget::on_pb_export_save_clicked()
{
    int projectId = ui->cb_export_project->currentData().toInt();
    QString docType = ui->cb_export_doc_type->currentData().toString();
    QString format = ui->cb_export_format->currentData().toString();
    QString projectName = ui->cb_export_project->currentText();

    if (projectId == 0) return;

    if (docType == "contract" && format == "csv") {
        QMessageBox::warning(this, "Внимание", "Формат CSV не подходит для текстового договора. Выберите PDF или HTML.");
        return;
    }

    QString defaultFilter;
    if (format == "pdf") defaultFilter = "PDF (*.pdf)";
    else if (format == "html") defaultFilter = "HTML (*.html)";
    else if (format == "csv") defaultFilter = "CSV (*.csv)";

    QString defaultFileName = QString("%1_%2.%3")
                                  .arg(docType == "estimate" ? "Smeta" : "Dogovor")
                                  .arg(projectName.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_"))
                                  .arg(format);

    QString savePath = QFileDialog::getSaveFileName(this, "Сохранить документ", defaultFileName, defaultFilter);
    if (savePath.isEmpty()) return;

    if (format == "pdf") {
        QPdfWriter pdfWriter(savePath);
        pdfWriter.setPageSize(QPageSize(QPageSize::A4));
        pdfWriter.setResolution(300);
        QMarginsF margins(15, 15, 15, 15);
        pdfWriter.setPageMargins(margins, QPageLayout::Millimeter);

        QTextDocument doc;
        doc.setHtml(ui->tb_export_preview->toHtml());
        doc.print(&pdfWriter);

    } else if (format == "html") {
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);
            out << ui->tb_export_preview->toHtml();
            file.close();
        }
    } else if (format == "csv") {
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out.setEncoding(QStringConverter::Utf8);
            out.setGenerateByteOrderMark(true);

            QSettings settings(QCoreApplication::applicationDirPath() + "/config.ini", QSettings::IniFormat);
            QString compName = settings.value("Company/Name", "ООО «Строительная Компания»").toString();
            QString compAddress = settings.value("Company/Address", "—").toString();
            QString compPhone = settings.value("Company/Phone", "—").toString();

            out << "Компания:;" << compName << "\n";
            out << "Юр. адрес:;" << compAddress << "\n";
            out << "Тел.:;" << compPhone << "\n";
            out << "\n";

            out << "Группа;Наименование;Количество;Цена за ед.;Сумма\n";

            QSqlDatabase db = QSqlDatabase::database();
            QSqlQuery query(db);
            query.prepare("SELECT pe.group_type, m.name, pe.quantity, pe.price_per_unit, (pe.quantity * pe.price_per_unit) AS total "
                          "FROM project_estimates pe "
                          "LEFT JOIN materials m ON pe.material_id = m.id "
                          "WHERE pe.project_id = :id "
                          "ORDER BY pe.group_type, m.name");
            query.bindValue(":id", projectId);

            QLocale loc(QLocale::Russian, QLocale::Russia);
            if (query.exec()) {
                while (query.next()) {
                    QString group = query.value("group_type").toString();
                    QString name = query.value("name").toString();
                    QString qty = loc.toString(query.value("quantity").toDouble(), 'f', 3);
                    QString price = loc.toString(query.value("price_per_unit").toDouble(), 'f', 2);
                    QString total = loc.toString(query.value("total").toDouble(), 'f', 2);

                    qty.replace(loc.groupSeparator(), "");
                    price.replace(loc.groupSeparator(), "");
                    total.replace(loc.groupSeparator(), "");

                    out << QString("\"%1\";\"%2\";\"%3\";\"%4\";\"%5\"\n")
                               .arg(group, name, qty, price, total);
                }
            }
            file.close();
        }
    }

    QMessageBox::information(this, "Успех", "Документ успешно сохранён!");
    QDesktopServices::openUrl(QUrl::fromLocalFile(savePath));
}

void MainWidget::on_pb_save_settings_clicked()
{
    saveCompanySettings();
}

void MainWidget::on_pb_select_logo_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите логотип", "", "Изображения (*.png *.jpg *.jpeg)");
    if (filePath.isEmpty()) {
        return;
    }

    QString settingsFolder = FileStorageManager::getStorageRoot() + "/settings";
    FileStorageManager::ensureFolderExists(settingsFolder);

    QFileInfo fileInfo(filePath);
    QString targetPath = settingsFolder + "/" + fileInfo.fileName();

    if (QDir::toNativeSeparators(filePath) != QDir::toNativeSeparators(targetPath)) {
        if (QFile::exists(targetPath)) {
            QFile::remove(targetPath);
        }

        if (!QFile::copy(filePath, targetPath)) {
            QMessageBox::critical(this, "Ошибка", "Не удалось скопировать логотип в локальное хранилище.");
            return;
        }
    }

    QDir appDir(QCoreApplication::applicationDirPath());
    m_companyLogoPath = appDir.relativeFilePath(targetPath);

    QPixmap pixmap(targetPath);
    ui->lb_logo->setPixmap(pixmap.scaled(ui->lb_logo->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void MainWidget::on_pb_open_editor_clicked()
{
    int currentRow = ui->tw_projects_list->currentRow();
    if (currentRow < 0) {
        QMessageBox::warning(this, "Внимание", "Пожалуйста, выберите проект из списка.");
        return;
    }

    QTableWidgetItem *item = ui->tw_projects_list->item(currentRow, 0);
    if (!item) {
        return;
    }

    int projectId = item->data(Qt::UserRole).toInt();

    EditorWindow *editor = new EditorWindow(projectId, this);
    editor->setAttribute(Qt::WA_DeleteOnClose);
    editor->show();
}
