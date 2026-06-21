#include "estimatedialog.h"
#include "ui_estimatedialog.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMap>
#include <QDebug>
#include "wallitem.h"
#include "flooritem.h"
#include "roofitem.h"
#include "windowitem.h"
#include "dooritem.h"
#include "foundationblockitem.h"
#include "nodeitem.h"
#include "objectitem.h"

EstimateDialog::EstimateDialog(const QList<BaseEditorItem*> &items, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EstimateDialog)
{
    ui->setupUi(this);
    setWindowTitle("Смета проекта");
    resize(800, 500);

    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels({"Наименование материала", "Ед. изм.", "Кол-во", "Цена за ед.", "Сумма"});
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QHeaderView *header = ui->tableWidget->horizontalHeader();

    header->setSectionResizeMode(0, QHeaderView::Stretch);

    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(4, QHeaderView::ResizeToContents);

    calculateEstimate(items);
}

EstimateDialog::~EstimateDialog()
{
    delete ui;
}

void EstimateDialog::on_btn_close_clicked()
{
    accept();
}

double EstimateDialog::extractQuantityByUnit(BaseEditorItem *item, const QString &unit)
{
    if (unit == "шт" || unit == "комплект") return 1.0;

    if (WallItem *wall = dynamic_cast<WallItem*>(item)) {
        if (unit == "м³") return wall->netVolume();
        if (unit == "м²") return wall->netSurfaceArea();
        if (unit == "м") return wall->lengthInMeters();
    }
    else if (FloorItem *floor = dynamic_cast<FloorItem*>(item)) {
        if (unit == "м³") return floor->volume();
        if (unit == "м²") return floor->area();
    }
    else if (RoofItem *roof = dynamic_cast<RoofItem*>(item)) {
        if (unit == "м³") return roof->volume();
        if (unit == "м²") return roof->area();
    }
    else if (FoundationBlockItem *found = dynamic_cast<FoundationBlockItem*>(item)) {
        if (unit == "м³") return found->volume();
        if (unit == "м²") return found->area();
    }
    else if (WindowItem *win = dynamic_cast<WindowItem*>(item)) {
        if (unit == "м²") return win->area();
    }
    else if (DoorItem *door = dynamic_cast<DoorItem*>(item)) {
        if (unit == "м²") return door->area();
    }
    else if (NodeItem *node = dynamic_cast<NodeItem*>(item)) {
        if (unit == "м³") return node->area() * node->maxAttachedWallHeight();
    }

    return 0.0;
}

void EstimateDialog::calculateEstimate(const QList<BaseEditorItem*> &items)
{
    QMap<int, EstimateRow> estimateData;

    QSqlQuery query;
    if (query.exec("SELECT m.id, m.name, m.base_price, u.name as unit_name "
                   "FROM materials m "
                   "JOIN units u ON m.unit_id = u.id")) {
        while (query.next()) {
            int id = query.value("id").toInt();
            EstimateRow row;
            row.name = query.value("name").toString();
            row.price = query.value("base_price").toDouble();
            row.unit = query.value("unit_name").toString();
            row.quantity = 0.0;

            estimateData.insert(id, row);
        }
    }

    QMap<QString, int> defaultMaterials;
    if (query.exec("SELECT c.system_code, MIN(m.id) FROM categories c JOIN materials m ON m.category_id = c.id GROUP BY c.system_code")) {
        while (query.next()) {
            defaultMaterials[query.value(0).toString()] = query.value(1).toInt();
        }
    }

    for (BaseEditorItem *item : items) {
        int matId = item->materialId();

        // === ИСПРАВЛЕНИЕ: Добавлен NodeItem (Узлы/Углы стен), чтобы их объем не терялся ===
        if (matId == -1) {
            if (dynamic_cast<FoundationBlockItem*>(item)) matId = defaultMaterials["FOUNDATION_MAT"];
            else if (dynamic_cast<WallItem*>(item)) matId = defaultMaterials["WALL_MAT"];
            else if (dynamic_cast<FloorItem*>(item)) matId = defaultMaterials["FLOOR_MAT"];
            else if (dynamic_cast<RoofItem*>(item)) matId = defaultMaterials["ROOF_MAT"];
            else if (dynamic_cast<WindowItem*>(item)) matId = defaultMaterials["WINDOW_MAT"];
            else if (dynamic_cast<DoorItem*>(item)) matId = defaultMaterials["DOOR_MAT"];
            else if (dynamic_cast<NodeItem*>(item)) matId = defaultMaterials["WALL_MAT"];
        }

        if (matId != -1 && estimateData.contains(matId)) {
            QString unit = estimateData[matId].unit;
            double qty = extractQuantityByUnit(item, unit);

            estimateData[matId].quantity += qty;
        }
    }

    double totalProjectCost = 0.0;
    ui->tableWidget->setRowCount(0);

    // === ИСПРАВЛЕНИЕ: Используем русскую локаль для идеального совпадения стилей ===
    QLocale loc(QLocale::Russian, QLocale::Russia);

    for (auto it = estimateData.begin(); it != estimateData.end(); ++it) {
        if (it.value().quantity > 0.0001) {
            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);

            double sum = it.value().quantity * it.value().price;
            totalProjectCost += sum;

            // Наименование
            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(it.value().name));

            // Ед. изм. (По центру)
            QTableWidgetItem *unitItem = new QTableWidgetItem(it.value().unit);
            unitItem->setTextAlignment(Qt::AlignCenter);
            ui->tableWidget->setItem(row, 1, unitItem);

            // Количество
            QString qtyStr = (it.value().unit == "шт") ? QString::number(it.value().quantity, 'f', 0) : QString::number(it.value().quantity, 'f', 3);
            QTableWidgetItem *qtyItem = new QTableWidgetItem(qtyStr);
            qtyItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->tableWidget->setItem(row, 2, qtyItem);

            // Цена (форматированная с ₽)
            QTableWidgetItem *priceItem = new QTableWidgetItem(loc.toString(it.value().price, 'f', 2) + " ₽");
            priceItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            ui->tableWidget->setItem(row, 3, priceItem);

            // Сумма (жирная, форматированная с ₽)
            QTableWidgetItem *sumItem = new QTableWidgetItem(loc.toString(sum, 'f', 2) + " ₽");
            sumItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
            sumItem->setFont(QFont("Segoe UI", -1, QFont::Bold));
            ui->tableWidget->setItem(row, 4, sumItem);
        }
    }

    // Итоговая сумма
    ui->lbl_total->setText(QString("ИТОГО ПО ПРОЕКТУ: %1").arg(loc.toString(totalProjectCost, 'f', 2) + " ₽"));
}
