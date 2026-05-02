#include "mainwidget.h"
#include "ui_mainwidget.h"

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWidget)
{
    ui->setupUi(this);

    ui->splitter->setSizes({150, 800});

    ui->sw_main->setCurrentIndex(0);

    connect(ui->lw_main, &QListWidget::currentRowChanged, this, &MainWidget::sw_main_change);
    connect(ui->splitter_4, &QSplitter::splitterMoved, this, &MainWidget::updateCatalogLayout);

    fillDemoClients();
    fillDemoClientDetails();
    fillDemoClientProjects();
    fillDemoClientFinance();
    fillDemoClientFiles();
    setupProjectFilters();
    fillDemoProjectsTable();
    fillDemoProjectDetail();
    fillDemoProjectEstimate();
    fillDemoProjectFiles();
    setupCatalogFilters();
    fillDemoCatalog();
}

MainWidget::~MainWidget()
{
    delete ui;
}

void MainWidget::sw_main_change(int index)
{
    if (index < 0) return;

    ui->sw_main->setCurrentIndex(index);

    switch (index) {
    case 0:
        qDebug() << "Главная";
        break;

    case 1:
        qDebug() << "Клиенты";
        break;

    case 2:
        qDebug() << "Проекты";
        break;

    case 3:
        qDebug() << "Каталог";
        break;

    case 4:
        qDebug() << "Экспорт";
        break;

    case 5:
        qDebug() << "Администрирование";
        break;

    default:
        break;
    }
}

void MainWidget::fillDemoClients()
{
    ui->tw_clients->setColumnCount(2);
    ui->tw_clients->setHorizontalHeaderLabels({"ФИО / Организация", "Номер телефона"});

    // Настраиваем колонки: ФИО растягивается, телефон по размеру текста
    ui->tw_clients->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_clients->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    // Очищаем старое (если было)
    ui->tw_clients->setRowCount(0);

    // 2. Список тестовых данных
    struct DemoData { QString fio; QString phone; };
    QList<DemoData> demoList = {
        {"Иванов Иван Иванович", "+7 (900) 123-45-67"},
        {"Петров Петр Петрович", "+7 (911) 222-33-44"},
        {"Сидорова Мария Александровна", "+7 (950) 888-11-22"},
        {"ООО 'ГлавСтройИнвест'", "+7 (495) 777-00-00"},
        {"Васильев Сергей Викторович", "+7 (905) 111-22-33"},
        {"Смирнова Анна Сергеевна", "+7 (960) 444-55-66"},
        {"ЗАО 'ТехноПарк'", "+7 (812) 333-22-11"},
        {"Кузнецов Дмитрий Олегович", "+7 (999) 000-01-02"}
    };

    // 3. Заполнение таблицы
    for (int i = 0; i < demoList.size(); ++i) {
        ui->tw_clients->insertRow(i);

        // Создаем ячейку ФИО
        QTableWidgetItem *itemFio = new QTableWidgetItem(demoList[i].fio);
        // Добавляем иконку (опционально, если есть в ресурсах, выглядит круто)
        // itemFio->setIcon(QIcon(":/res/user_icon.png"));

        // Создаем ячейку Телефона
        QTableWidgetItem *itemPhone = new QTableWidgetItem(demoList[i].phone);

        // Выравнивание для красоты
        itemPhone->setTextAlignment(Qt::AlignCenter);

        // Устанавливаем в таблицу
        ui->tw_clients->setItem(i, 0, itemFio);
        ui->tw_clients->setItem(i, 1, itemPhone);
    }

    // Разрешаем выделение только строк
    ui->tw_clients->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_clients->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tw_clients->setEditTriggers(QAbstractItemView::NoEditTriggers); // Запрещаем правку
}

void MainWidget::fillDemoClientDetails()
{
    ui->lb_clients_clientName->setText("Иванов Иван Иванович");
    ui->le_clients_clientPhone->setText("+7 (900) 123-45-67");
    ui->le_clients_clientEmail->setText("ivanovivan@gmail.com");
    ui->le_clients_clientAddress->setText("Город, улица, дом");
    ui->le_clients_clientPasport->setText("1234 123456");
    ui->pte_clients_clientNotes->setPlainText("Пример описания");
}

void MainWidget::fillDemoClientProjects()
{
    // 1. Настройка колонок (Название, Статус, Этап, Готовность, Сумма)
    ui->tw_client_projects->setColumnCount(5);
    ui->tw_client_projects->setHorizontalHeaderLabels({
        "Объект", "Статус", "Текущий этап", "Готовность", "Сумма договора"
    });

    // Растягиваем первую колонку с названием, остальные по контенту
    ui->tw_client_projects->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    for(int i = 1; i < 5; ++i)
        ui->tw_client_projects->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);

    ui->tw_client_projects->setRowCount(0);

    // 2. Тестовые данные
    struct ProjDemo {
        QString name;
        QString status;
        QString stage;
        QString progress;
        QString total;
        QColor statusColor;
    };

    QList<ProjDemo> projects = {
        {"Коттедж 'Скандинавия' (участок 14)", "Строительство", "Кровля", "65%", "8 450 000 руб.", QColor(0, 120, 215)},
        {"Гостевой дом с баней", "Завершено", "Объект сдан", "100%", "2 100 000 руб.", QColor(0, 150, 0)},
        {"Гараж на 2 автомобиля", "Проектирование", "Архитектурный план", "20%", "950 000 руб.", QColor(255, 140, 0)}
    };

    // 3. Заполнение
    for (int i = 0; i < projects.size(); ++i) {
        ui->tw_client_projects->insertRow(i);

        // Объект
        ui->tw_client_projects->setItem(i, 0, new QTableWidgetItem(projects[i].name));

        // Статус (с фоновым цветом для наглядности на скриншоте)
        QTableWidgetItem *statusItem = new QTableWidgetItem(projects[i].status);
        statusItem->setBackground(projects[i].statusColor);
        statusItem->setForeground(Qt::white); // Белый текст на цветном фоне
        statusItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_client_projects->setItem(i, 1, statusItem);

        // Этап
        ui->tw_client_projects->setItem(i, 2, new QTableWidgetItem(projects[i].stage));

        // Готовность
        QTableWidgetItem *progItem = new QTableWidgetItem(projects[i].progress);
        progItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_client_projects->setItem(i, 3, progItem);

        // Сумма
        QTableWidgetItem *totalItem = new QTableWidgetItem(projects[i].total);
        totalItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        totalItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
        ui->tw_client_projects->setItem(i, 4, totalItem);
    }

    // Тонкая настройка стиля
    ui->tw_client_projects->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_client_projects->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tw_client_projects->verticalHeader()->setVisible(false); // Скрываем номера строк слева
}

void MainWidget::fillDemoClientFinance()
{
    // 1. Заполнение итоговых показателей (Суммарные лейблы)
    double totalContract = 11500000.0;
    double totalPaid = 7500000.0;
    double debt = totalContract - totalPaid;

    // Форматируем числа с разделением разрядов (через пробел) для читаемости
    ui->lb_total_contract_sum->setText(QString::number(totalContract, 'f', 0) + " ₽");
    ui->lb_paid_sum->setText(QString::number(totalPaid, 'f', 0) + " ₽");
    ui->lb_debt_sum->setText(QString::number(debt, 'f', 0) + " ₽");

    // Выделяем долг красным цветом, если он больше нуля
    if (debt > 0) {
        ui->lb_debt_sum->setStyleSheet("color: #D32F2F; font-weight: bold; font-size: 14pt;");
    } else {
        ui->lb_debt_sum->setStyleSheet("color: #388E3C; font-weight: bold; font-size: 14pt;");
    }

    // 2. Настройка таблицы платежей (tw_payments)
    ui->tw_payments->setColumnCount(4);
    ui->tw_payments->setHorizontalHeaderLabels({
        "Дата", "Сумма", "Назначение", "Статус"
    });

    ui->tw_payments->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_payments->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_payments->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    ui->tw_payments->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);

    ui->tw_payments->setRowCount(0);

    // 3. Тестовые данные для истории платежей
    struct PaymentDemo {
        QString date;
        QString amount;
        QString purpose;
        QString status;
        QColor statusColor;
    };

    QList<PaymentDemo> payments = {
        {"10.02.2026", "4 225 000 ₽", "Аванс: Коттедж 'Скандинавия' (50%)", "Зачислен", QColor(56, 142, 60)},
        {"01.03.2026", "2 100 000 ₽", "Полная оплата: Гостевой дом с баней", "Зачислен", QColor(56, 142, 60)},
        {"15.03.2026", "950 000 ₽", "Оплата этапа 'Фундамент' (Алюминиевый профиль)", "Зачислен", QColor(56, 142, 60)},
        {"20.03.2026", "225 000 ₽", "Предоплата: Проектирование гаража", "В обработке", QColor(25, 118, 210)}
    };

    // 4. Заполнение таблицы
    for (int i = 0; i < payments.size(); ++i) {
        ui->tw_payments->insertRow(i);

        // Дата
        ui->tw_payments->setItem(i, 0, new QTableWidgetItem(payments[i].date));

        // Сумма (вправо)
        QTableWidgetItem *amountItem = new QTableWidgetItem(payments[i].amount);
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        amountItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
        ui->tw_payments->setItem(i, 1, amountItem);

        // Назначение
        ui->tw_payments->setItem(i, 2, new QTableWidgetItem(payments[i].purpose));

        // Статус
        QTableWidgetItem *statusItem = new QTableWidgetItem(payments[i].status);
        statusItem->setForeground(payments[i].statusColor); // Цветной текст статуса
        statusItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_payments->setItem(i, 3, statusItem);
    }

    // Настройки внешнего вида таблицы
    ui->tw_payments->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_payments->verticalHeader()->setVisible(false);
    ui->tw_payments->setAlternatingRowColors(true);
}

void MainWidget::fillDemoClientFiles()
{
    // 1. Базовая настройка ListWidget
    ui->lw_client_files->clear();
    ui->lw_client_files->setIconSize(QSize(32, 32)); // Делаем иконки крупными и заметными
    ui->lw_client_files->setSpacing(5);             // Добавляем отступы между элементами

    // 2. Тестовые данные (Имя файла, тип/расширение, дата добавления)
    struct FileDemo {
        QString name;
        QString type; // pdf, docx, xlsx, dwg, jpg
        QString date;
    };

    QList<FileDemo> files = {
        {"Договор_подряда_№45-А.pdf", "pdf", "10.02.2026"},
        {"Техническое_задание_v2.docx", "docx", "12.02.2026"},
        {"Смета_материалов_фундамент.xlsx", "xlsx", "15.02.2026"},
        {"План_участка_кадастр.pdf", "pdf", "10.02.2026"},
        {"Чертеж_фасада_основной.dwg", "dwg", "01.03.2026"},
        {"Фото_площадки_до_начала_работ.jpg", "jpg", "05.03.2026"},
        {"Акт_приемки_скрытых_работ.pdf", "pdf", "20.03.2026"}
    };

    // 3. Заполнение списка
    for (const auto &file : files) {
        // Создаем текст: Имя файла + дата в скобках
        QString displayText = QString("%1\n(%2)").arg(file.name, file.date);
        QListWidgetItem *item = new QListWidgetItem(displayText, ui->lw_client_files);

        // 4. Установка иконки в зависимости от типа файла
        // В реальном проекте здесь будут пути к твоим .png иконкам в ресурсах (:/res/pdf.png)
        // Для демо используем стандартные системные иконки Qt, чтобы код сразу заработал
        QIcon fileIcon;
        if (file.type == "pdf")
            fileIcon = style()->standardIcon(QStyle::SP_FileIcon); // Замени на иконку PDF
        else if (file.type == "xlsx" || file.type == "docx")
            fileIcon = style()->standardIcon(QStyle::SP_FileIcon); // Замени на иконку Office
        else if (file.type == "jpg")
            fileIcon = style()->standardIcon(QStyle::SP_FileDialogContentsView); // Замени на иконку Image
        else
            fileIcon = style()->standardIcon(QStyle::SP_FileIcon); // Общая иконка

        item->setIcon(fileIcon);

        // Добавляем подсказку при наведении
        item->setToolTip("Нажмите дважды, чтобы открыть файл");

        ui->lw_client_files->addItem(item);
    }
}

void MainWidget::setupProjectFilters()
{
    ui->cb_project_status_filter->clear();
    ui->cb_project_status_filter->addItem("📊 Все статусы");
    ui->cb_project_status_filter->addItem("✏️ Проектирование");
    ui->cb_project_status_filter->addItem("🏗️ Строительство");
    ui->cb_project_status_filter->addItem("✅ Завершено");
    ui->cb_project_status_filter->addItem("❄️ Заморожено");
}

void MainWidget::fillDemoProjectsTable()
{
    // 1. Настройка колонок
    ui->tw_projects_list->setColumnCount(6);
    ui->tw_projects_list->setHorizontalHeaderLabels({
        "Название проекта", "Заказчик", "Статус", "Начало", "План. завершение", "Бюджет"
    });

    // Растягиваем название проекта и имя заказчика
    ui->tw_projects_list->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_projects_list->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tw_projects_list->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_projects_list->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);

    ui->tw_projects_list->setRowCount(0);

    // 2. Тестовые данные
    struct ProjectEntry {
        QString name;
        QString client;
        QString status;
        QString startDate;
        QString endDate;
        QString budget;
        QColor statusColor;
    };

    QList<ProjectEntry> demoProjects = {
        {"Коттедж 'Скандинавия'", "Иванов И.И.", "Строительство", "10.01.2026", "15.11.2026", "8 450 000 ₽", QColor(0, 120, 215)},
        {"Гостевой дом с баней", "Петров П.П.", "Завершено", "15.11.2025", "01.03.2026", "2 100 000 ₽", QColor(56, 142, 60)},
        {"Гараж на 2 авто", "ООО 'ГлавСтрой'", "Проектирование", "20.03.2026", "10.05.2026", "950 000 ₽", QColor(255, 140, 0)},
        {"Торговый павильон", "ЗАО 'ТехноПарк'", "Заморожено", "01.12.2025", "30.06.2026", "4 500 000 ₽", QColor(120, 120, 120)},
        {"Реконструкция мансарды", "Смирнова А.С.", "Строительство", "05.03.2026", "20.04.2026", "1 200 000 ₽", QColor(0, 120, 215)}
    };

    // 3. Заполнение таблицы
    for (int i = 0; i < demoProjects.size(); ++i) {
        ui->tw_projects_list->insertRow(i);

        // Название проекта
        ui->tw_projects_list->setItem(i, 0, new QTableWidgetItem(demoProjects[i].name));

        // Заказчик
        ui->tw_projects_list->setItem(i, 1, new QTableWidgetItem(demoProjects[i].client));

        // Статус (цветной текст)
        QTableWidgetItem *statusItem = new QTableWidgetItem(demoProjects[i].status);
        statusItem->setForeground(demoProjects[i].statusColor);
        statusItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
        statusItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_projects_list->setItem(i, 2, statusItem);

        // Даты
        ui->tw_projects_list->setItem(i, 3, new QTableWidgetItem(demoProjects[i].startDate));
        ui->tw_projects_list->setItem(i, 4, new QTableWidgetItem(demoProjects[i].endDate));

        // Бюджет
        QTableWidgetItem *budgetWeight = new QTableWidgetItem(demoProjects[i].budget);
        budgetWeight->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tw_projects_list->setItem(i, 5, budgetWeight);
    }

    // Общие настройки таблицы
    ui->tw_projects_list->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_projects_list->setAlternatingRowColors(true);
    ui->tw_projects_list->verticalHeader()->setVisible(false);
    ui->tw_projects_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void MainWidget::fillDemoProjectDetail()
{
    // 1. Заполнение основных меток и полей
    ui->lb_proj_name->setText("Гараж на 2 автомобиля (Индивидуальный проект)");
    ui->lb_proj_client_ref->setText("Заказчик: ООО 'ГлавСтрой'");
    ui->lb_proj_client_ref->setStyleSheet("color: blue; text-decoration: underline; font-weight: bold;");

    // Статус (используем QLineEdit как информационное поле)
    ui->le_proj_status->setText("Проектирование");
    ui->le_proj_status->setReadOnly(true);
    ui->le_proj_status->setStyleSheet("background-color: #FFF3E0; color: #E65100; font-weight: bold; padding: 5px;");

    // 2. Описание проекта (QPlainTextEdit)
    QString description =
        "Тип объекта: Капитальное строение\n"
        "Стиль: Хай-тек\n"
        "Площадь: 48 м² (6х8 м)\n"
        "Материал стен: Газобетонные блоки\n"
        "Кровля: Плоская с системой внутреннего водостока\n"
        "Особенности: Установка двух автоматических ворот, наличие смотровой ямы, "
        "зона для хранения инструментов.";
    ui->pte_project_description->setPlainText(description);
    ui->pte_project_description->setReadOnly(true);

    // 3. Настройка таблицы этапов (tw_project_stages)
    ui->tw_project_stages->setColumnCount(5);
    ui->tw_project_stages->setHorizontalHeaderLabels({
        "Этап работ", "План. дата", "Факт. дата", "Ответственный", "Статус"
    });

    ui->tw_project_stages->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tw_project_stages->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->tw_project_stages->setRowCount(0);

    // Тестовые этапы для гаража
    struct StageDemo {
        QString name;
        QString planDate;
        QString factDate;
        QString leader;
        QString status;
        QColor statusColor;
    };

    QList<StageDemo> stages = {
        {"Замеры и геодезия участка", "20.03.2026", "21.03.2026", "Котов А.В.", "✅ Готово", QColor(56, 142, 60)},
        {"Архитектурный план (АР)", "25.03.2026", "28.03.2026", "Смирнова А.С.", "✅ Готово", QColor(56, 142, 60)},
        {"Расчет фундамента и нагрузок", "02.04.2026", "-", "Петров П.П.", "🏗️ В работе", QColor(25, 118, 210)},
        {"Подбор материалов и смета", "10.04.2026", "-", "Администратор", "⏳ Ожидание", QColor(120, 120, 120)},
        {"Заливка плиты основания", "20.04.2026", "-", "Бригада №3", "⏳ Ожидание", QColor(120, 120, 120)}
    };

    // 4. Заполнение таблицы
    for (int i = 0; i < stages.size(); ++i) {
        ui->tw_project_stages->insertRow(i);

        ui->tw_project_stages->setItem(i, 0, new QTableWidgetItem(stages[i].name));
        ui->tw_project_stages->setItem(i, 1, new QTableWidgetItem(stages[i].planDate));
        ui->tw_project_stages->setItem(i, 2, new QTableWidgetItem(stages[i].factDate));
        ui->tw_project_stages->setItem(i, 3, new QTableWidgetItem(stages[i].leader));

        QTableWidgetItem *statusItem = new QTableWidgetItem(stages[i].status);
        statusItem->setForeground(stages[i].statusColor);
        statusItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
        ui->tw_project_stages->setItem(i, 4, statusItem);
    }

    // Дополнительные визуальные настройки
    ui->tw_project_stages->setAlternatingRowColors(true);
    ui->tw_project_stages->verticalHeader()->setVisible(false);
    ui->tw_project_stages->setSelectionBehavior(QAbstractItemView::SelectRows);

    ui->pb_stage_progress->setValue(40); // 2 этапа из 5 готовы
}

void MainWidget::fillDemoProjectEstimate()
{
    // 1. Настройка таблицы сметы (tw_project_estimate)
    ui->tw_project_estimate->setColumnCount(6);
    ui->tw_project_estimate->setHorizontalHeaderLabels({
        "Наименование / Группа", "Ед. изм.", "Кол-во", "Цена за ед.", "Сумма", "Статус"
    });

    // Настройка размеров колонок
    ui->tw_project_estimate->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch); // Название тянется
    ui->tw_project_estimate->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->tw_project_estimate->setRowCount(0);

    // 2. Структура для демо-данных
    struct EstimateItem {
        QString name;
        QString unit;
        double qty;
        double price;
        QString status;
        QColor statusColor;
        bool isHeader; // Флаг для выделения заголовков категорий
    };

    QList<EstimateItem> items = {
        {"МАТЕРИАЛЫ", "", 0, 0, "", Qt::black, true},
        {"Бетон М300 (фундамент)", "м³", 12.5, 6200, "Оплачено", QColor(56, 142, 60), false},
        {"Газобетонный блок 600х300", "м³", 32.0, 5800, "Оплачено", QColor(56, 142, 60), false},
        {"Арматура 12мм", "тн", 0.85, 72000, "Ожидание", QColor(25, 118, 210), false},
        {"Кровельный профнастил", "м²", 55.0, 850, "Черновик", QColor(120, 120, 120), false},

        {"РАБОТЫ", "", 0, 0, "", Qt::black, true},
        {"Земляные работы и разметка", "услуга", 1, 15000, "Оплачено", QColor(56, 142, 60), false},
        {"Заливка фундаментной плиты", "услуга", 1, 85000, "В работе", QColor(255, 140, 0), false},
        {"Кладка стен", "м²", 96.0, 1200, "Черновик", QColor(120, 120, 120), false},

        {"ДОСТАВКА И ТЕХНИКА", "", 0, 0, "", Qt::black, true},
        {"Аренда бетононасоса", "смена", 1, 25000, "Оплачено", QColor(56, 142, 60), false},
        {"Доставка блоков (Манипулятор)", "рейс", 2, 8000, "Ожидание", QColor(25, 118, 210), false}
    };

    double grandTotal = 0;

    // 3. Заполнение таблицы
    for (int i = 0; i < items.size(); ++i) {
        ui->tw_project_estimate->insertRow(i);

        if (items[i].isHeader) {
            // Оформление строки-заголовка (МАТЕРИАЛЫ, РАБОТЫ...)
            QTableWidgetItem *headerItem = new QTableWidgetItem(items[i].name);
            headerItem->setBackground(QColor(240, 240, 240));
            headerItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
            ui->tw_project_estimate->setItem(i, 0, headerItem);

            // Объединяем ячейки в строке заголовка для красоты
            ui->tw_project_estimate->setSpan(i, 0, 1, 6);
            continue;
        }

        double rowSum = items[i].qty * items[i].price;
        grandTotal += rowSum;

        // Наименование
        ui->tw_project_estimate->setItem(i, 0, new QTableWidgetItem(items[i].name));

        // Ед. изм.
        QTableWidgetItem *unitItem = new QTableWidgetItem(items[i].unit);
        unitItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_project_estimate->setItem(i, 1, unitItem);

        // Кол-во
        QTableWidgetItem *qtyItem = new QTableWidgetItem(QString::number(items[i].qty, 'f', 1));
        qtyItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_project_estimate->setItem(i, 2, qtyItem);

        // Цена
        QTableWidgetItem *priceItem = new QTableWidgetItem(QString::number(items[i].price, 'f', 0) + " ₽");
        priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tw_project_estimate->setItem(i, 3, priceItem);

        // Сумма строки
        QTableWidgetItem *sumItem = new QTableWidgetItem(QString::number(rowSum, 'f', 0) + " ₽");
        sumItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        sumItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
        ui->tw_project_estimate->setItem(i, 4, sumItem);

        // Статус
        QTableWidgetItem *statusItem = new QTableWidgetItem(items[i].status);
        statusItem->setForeground(items[i].statusColor);
        statusItem->setTextAlignment(Qt::AlignCenter);
        ui->tw_project_estimate->setItem(i, 5, statusItem);
    }

    // 4. Обновление итогового лейбла (lb_total_estimate_sum)
    // Добавим к сумме налог или маржу 10% для реалистичности "итоговой стоимости объекта"
    double margin = grandTotal * 0.10;
    double finalSum = grandTotal + margin;

    ui->lb_total_estimate_sum->setText(QString::number(finalSum, 'f', 0) + " ₽");
    ui->lb_total_estimate_sum->setStyleSheet("font-size: 18pt; font-weight: bold; color: #1976D2;");

    // Общие настройки
    ui->tw_project_estimate->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tw_project_estimate->verticalHeader()->setVisible(false);
}

void MainWidget::fillDemoProjectFiles()
{
    // 1. Настройка ListWidget
    ui->lw_project_docs->clear();
    ui->lw_project_docs->setIconSize(QSize(40, 40)); // Иконки покрупнее для презентабельности
    ui->lw_project_docs->setSpacing(4);             // Небольшой отступ между пунктами

    // 2. Структура демо-данных
    struct FileItem {
        QString name;
        QString type; // Расширение
        QString size;
        QString date;
    };

    QList<FileItem> files = {
        {"АР_Гараж_ХайТек_Финальный.pdf", "pdf", "4.2 МБ", "20.03.2026"},
        {"План_фундамента_сетка.dwg", "dwg", "12.8 МБ", "22.03.2026"},
        {"Смета_материалов_полная.xlsx", "xlsx", "156 КБ", "25.03.2026"},
        {"Договор_подряда_№128_ГлСтрой.pdf", "pdf", "1.1 МБ", "20.03.2026"},
        {"Фото_привязки_к_местности.jpg", "jpg", "3.5 МБ", "21.03.2026"},
        {"Визуализация_день_рендер.png", "png", "8.2 МБ", "24.03.2026"},
        {"Техническое_задание_заказчика.docx", "docx", "45 КБ", "18.03.2026"}
    };

    // 3. Заполнение списка
    for (const auto &file : files) {
        // Формируем текст: Имя файла жирным (сверху) и доп. инфо (снизу)
        // \n создает вторую строку в элементе списка
        QString itemText = QString("%1\nРазмер: %2 | Добавлен: %3")
                               .arg(file.name)
                               .arg(file.size)
                               .arg(file.date);

        QListWidgetItem *item = new QListWidgetItem(itemText, ui->lw_project_docs);

        // 4. Установка иконок (используем стандартные или системные)
        QIcon icon;
        if (file.type == "pdf") {
            icon = style()->standardIcon(QStyle::SP_FileIcon); // Замени на иконку PDF красного цвета
        } else if (file.type == "xlsx" || file.type == "docx") {
            icon = style()->standardIcon(QStyle::SP_FileIcon); // Замени на иконку Office (синий/зеленый)
        } else if (file.type == "jpg" || file.type == "png") {
            icon = style()->standardIcon(QStyle::SP_FileDialogContentsView); // Иконка картинки
        } else if (file.type == "dwg") {
            icon = style()->standardIcon(QStyle::SP_FileIcon); // Иконка чертежа
        } else {
            icon = style()->standardIcon(QStyle::SP_FileIcon);
        }

        item->setIcon(icon);

        // Настраиваем шрифт: верхняя строка будет чуть крупнее
        QFont font = item->font();
        font.setPointSize(9);
        item->setFont(font);

        ui->lw_project_docs->addItem(item);
    }

    // 5. Визуальный стиль через код (или можно в QSS)
    ui->lw_project_docs->setStyleSheet(
        "QListWidget::item { border-bottom: 1px solid #f0f0f0; padding: 5px; }"
        "QListWidget::item:selected { background-color: #e3f2fd; color: #1976d2; border: none; }"
        );
}

void MainWidget::setupCatalogFilters()
{
    // Категории строений
    ui->cb_catalog_category->clear();
    ui->cb_catalog_category->addItems({
        "🏠 Все типы",
        "Брусовые дома",
        "Каменные дома",
        "Каркасные объекты",
        "Бани и сауны",
        "Гаражи и навесы"
    });

    // Этажность
    ui->cb_floors->clear();
    ui->cb_floors->addItems({"Любая этажность", "1 этаж", "2 этажа", "С мансардой"});

    // Тип кровли
    ui->cb_roof_type->clear();
    ui->cb_roof_type->addItems({"Любая кровля", "Двускатная", "Плоская", "Вальмовая"});
}

void MainWidget::fillDemoCatalog()
{
    // Очистка (полная, с удалением виджетов)
    if (ui->gridLayout_catalog) {
        QLayoutItem *item;
        while ((item = ui->gridLayout_catalog->takeAt(0)) != nullptr) {
            if (QWidget *w = item->widget()) w->deleteLater();
            delete item;
        }
    }

    struct CatalogItem { QString title; QString area; QString price; QString tags; };
    QList<CatalogItem> items = {
        {"Коттедж 'Осло'", "145 м²", "от 6.2 млн ₽", "2 этажа | Брус"},
        {"Дом 'Прага'", "110 м²", "от 4.8 млн ₽", "1 этаж | Кирпич"},
        {"Баня 'Уют'", "35 м²", "от 1.2 млн ₽", "1 этаж | Бревно"},
        {"Гараж 'Бокс-2'", "48 м²", "от 0.9 млн ₽", "Газобетон"},
        {"Вилла 'Майами'", "280 м²", "от 12.5 млн ₽", "2 этажа | Монолит"},
        {"Дачный дом 'Лето'", "60 м²", "от 2.1 млн ₽", "Мансарда | Каркас"}
    };

    // Создаем карточки, но пока не заботимся о row/col
    for (int i = 0; i < items.size(); ++i) {
        QFrame *card = new QFrame();
        card->setFixedSize(220, 280);
        card->setStyleSheet("QFrame { background-color: white; border: 1px solid #dcdcdc; border-radius: 8px; } "
                            "QFrame:hover { border: 2px solid #1976d2; background-color: #f5faff; }");

        QVBoxLayout *cardLayout = new QVBoxLayout(card);

        QLabel *imgLabel = new QLabel("НЕТ ФОТО");
        imgLabel->setFixedSize(200, 140);
        imgLabel->setAlignment(Qt::AlignCenter);
        imgLabel->setStyleSheet("background-color: #f0f0f0; border-radius: 4px;");

        QLabel *titleLabel = new QLabel(items[i].title);
        titleLabel->setStyleSheet("font-weight: bold; font-size: 11pt; border: none;");

        QLabel *tagsLabel = new QLabel(items[i].area + "\n" + items[i].tags);
        tagsLabel->setStyleSheet("color: #666; font-size: 9pt; border: none;");

        QLabel *priceLabel = new QLabel(items[i].price);
        priceLabel->setStyleSheet("color: #1976d2; font-weight: bold; border: none;");

        cardLayout->addWidget(imgLabel);
        cardLayout->addWidget(titleLabel);
        cardLayout->addWidget(tagsLabel);
        cardLayout->addStretch();
        cardLayout->addWidget(priceLabel);

        // Просто добавляем в лейаут, порядок исправит updateCatalogLayout
        ui->gridLayout_catalog->addWidget(card, 0, i);
    }

    // Применяем правильную расстановку
    updateCatalogLayout();
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
