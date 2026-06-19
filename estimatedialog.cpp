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

    for (BaseEditorItem *item : items) {
        int matId = item->materialId();
        if (matId != -1 && estimateData.contains(matId)) {
            QString unit = estimateData[matId].unit;
            double qty = extractQuantityByUnit(item, unit);

            estimateData[matId].quantity += qty;
        }
    }

    double totalProjectCost = 0.0;
    ui->tableWidget->setRowCount(0);

    for (auto it = estimateData.begin(); it != estimateData.end(); ++it) {
        if (it.value().quantity > 0.0001) {
            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);

            double sum = it.value().quantity * it.value().price;
            totalProjectCost += sum;

            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(it.value().name));
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(it.value().unit));

            QString qtyStr = (it.value().unit == "шт") ? QString::number(it.value().quantity, 'f', 0) : QString::number(it.value().quantity, 'f', 2);
            ui->tableWidget->setItem(row, 2, new QTableWidgetItem(qtyStr));

            ui->tableWidget->setItem(row, 3, new QTableWidgetItem(QString::number(it.value().price, 'f', 2)));
            ui->tableWidget->setItem(row, 4, new QTableWidgetItem(QString::number(sum, 'f', 2)));
        }
    }

    ui->lbl_total->setText(QString("ИТОГО ПО ПРОЕКТУ: %1 руб.").arg(totalProjectCost, 0, 'f', 2));
}
