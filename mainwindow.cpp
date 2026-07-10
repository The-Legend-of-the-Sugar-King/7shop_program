#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QAction>
#include <QHeaderView>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_ordersModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    initTables();
    connectUiSignals();

    // 订单明细表绑定 model
    m_ordersModel->setHorizontalHeaderLabels({"订单号", "时间", "件数", "金额", "状态"});
    ui->m_reportOrdersTable->setModel(m_ordersModel);
    ui->m_reportOrdersTable->horizontalHeader()->setStretchLastSection(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ============================================================
//  前端信号连接：只读取界面输入并发出业务请求
// ============================================================
void MainWindow::connectUiSignals()
{
    connect(ui->pushButton, &QPushButton::clicked, this, [this]() {
        emit searchByCodeRequested(ui->m_productCodeEdit->text().trimmed());
    });

    connect(ui->m_cashiersearchpushButton, &QPushButton::clicked, this, [this]() {
        emit cashierSearchRequested(ui->m_cashierSearchEdit->text().trimmed());
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, [this]() {
        emit addToCartRequested(ui->m_productCodeEdit->text().trimmed(),
                                ui->m_quantitySpin->value());
    });

    connect(ui->m_cashierProductTable, &QTableWidget::customContextMenuRequested,
            this, &MainWindow::showCashierProductMenu);

    connect(ui->m_cartTable, &QTableWidget::customContextMenuRequested,
            this, &MainWindow::showCartMenu);

    connect(ui->pushButton_6, &QPushButton::clicked, this, [this]() {
        emit checkoutRequested(ui->m_paidSpin->value());
    });

    connect(ui->pushButton_3, &QPushButton::clicked, this, [this]() {
        emit searchProductByIdRequested(ui->m_idEdit->text().trimmed());
    });

    connect(ui->pushButton_4, &QPushButton::clicked, this, [this]() {
        emit searchProductByNameRequested(ui->m_nameEdit->text().trimmed());
    });

    connect(ui->pushButton_5, &QPushButton::clicked, this, [this]() {
        emit searchProductByCategoryRequested(ui->m_categoryEdit->text().trimmed());
    });

    connect(ui->m_roleCombo,
            static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this,
            [this](int index) {
                emit roleChangedRequested(index == 0);
            });
}

void MainWindow::showCashierProductMenu(const QPoint &pos)
{
    QTableWidget *table = ui->m_cashierProductTable;
    QTableWidgetItem *item = table->itemAt(pos);
    if (!item) return;

    int row = item->row();
    QTableWidgetItem *codeItem = table->item(row, 0);
    if (!codeItem) return;

    QString code = codeItem->text();
    int qty = ui->m_quantitySpin->value();

    QMenu menu;
    QAction *addAction = menu.addAction(QString("加入购物车（%1 件）").arg(qty));
    QAction *chosen = menu.exec(table->viewport()->mapToGlobal(pos));

    if (chosen == addAction) {
        emit addToCartRequested(code, qty);
    }
}

void MainWindow::showCartMenu(const QPoint &pos)
{
    QTableWidget *cart = ui->m_cartTable;
    QTableWidgetItem *item = cart->itemAt(pos);
    if (!item) return;

    int row = item->row();

    QMenu menu;
    QAction *removeAction = menu.addAction("移出购物车");
    QAction *chosen = menu.exec(cart->viewport()->mapToGlobal(pos));

    if (chosen == removeAction) {
        emit removeCartItemRequested(row);
    }
}

// ============================================================
//  控件 getter
// ============================================================

QComboBox* MainWindow::roleCombo() const       { return ui->m_roleCombo; }

QLineEdit*     MainWindow::productCodeEdit() const    { return ui->m_productCodeEdit; }
QPushButton*   MainWindow::searchByCodeButton() const  { return ui->pushButton; }
QLineEdit*     MainWindow::cashierSearchEdit() const   { return ui->m_cashierSearchEdit; }
QPushButton*   MainWindow::cashierSearchButton() const { return ui->m_cashiersearchpushButton; }
QSpinBox*      MainWindow::quantitySpin() const        { return ui->m_quantitySpin; }

QTableWidget*  MainWindow::cashierProductTable() const { return ui->m_cashierProductTable; }
QTableWidget*  MainWindow::cartTable() const           { return ui->m_cartTable; }

QLabel*        MainWindow::cartCountLabel() const      { return ui->m_cartCountLabel; }
QLabel*        MainWindow::cartTotalLabel() const      { return ui->m_cartTotalLabel; }
QDoubleSpinBox* MainWindow::paidSpin() const           { return ui->m_paidSpin; }
QComboBox*     MainWindow::suspendedCombo() const      { return ui->m_suspendedCombo; }

QPushButton*   MainWindow::addCartButton() const       { return ui->pushButton_2; }
QPushButton*   MainWindow::checkoutButton() const      { return ui->pushButton_6; }

QTableWidget*  MainWindow::productTable() const        { return ui->m_productTable; }
QLineEdit*     MainWindow::idEdit() const              { return ui->m_idEdit; }
QPushButton*   MainWindow::searchByIdButton() const    { return ui->pushButton_3; }
QLineEdit*     MainWindow::nameEdit() const            { return ui->m_nameEdit; }
QPushButton*   MainWindow::searchByNameButton() const  { return ui->pushButton_4; }
QLineEdit*     MainWindow::categoryEdit() const        { return ui->m_categoryEdit; }
QPushButton*   MainWindow::searchByCategoryButton() const { return ui->pushButton_5; }
QDoubleSpinBox* MainWindow::priceSpin() const          { return ui->m_priceSpin; }
QSpinBox*      MainWindow::stockSpin() const           { return ui->m_stockSpin; }
QComboBox*     MainWindow::activeCombo() const         { return ui->m_activeCombo; }

QTableView*    MainWindow::reportOrdersTable() const   { return ui->m_reportOrdersTable; }
QTableWidget*  MainWindow::reportItemsTable() const    { return ui->m_reportltermsTable; }

// ============================================================
//  表格初始化
// ============================================================
void MainWindow::initTables()
{
    // 收银台商品表
    QStringList cashierHeaders = {"编号", "名称", "类别", "价格", "库存", "状态"};
    ui->m_cashierProductTable->setColumnCount(6);
    ui->m_cashierProductTable->setHorizontalHeaderLabels(cashierHeaders);
    ui->m_cashierProductTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->m_cashierProductTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->m_cashierProductTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->m_cashierProductTable->horizontalHeader()->setStretchLastSection(true);

    // 购物车表
    QStringList cartHeaders = {"编号", "名称", "单价", "数量", "小计"};
    ui->m_cartTable->setColumnCount(5);
    ui->m_cartTable->setHorizontalHeaderLabels(cartHeaders);
    ui->m_cartTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->m_cartTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->m_cartTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->m_cartTable->horizontalHeader()->setStretchLastSection(true);

    // 商品管理表
    QStringList productHeaders = {"编号", "名称", "类别", "价格", "库存", "状态"};
    ui->m_productTable->setColumnCount(6);
    ui->m_productTable->setHorizontalHeaderLabels(productHeaders);
    ui->m_productTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->m_productTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->m_productTable->horizontalHeader()->setStretchLastSection(true);

    // 订单明细表 (QTableView + model 在构造函数里设)
    ui->m_reportOrdersTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->m_reportOrdersTable->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 商品销量表
    QStringList salesHeaders = {"商品", "编号", "已售", "库存", ""};
    ui->m_reportltermsTable->setColumnCount(5);
    ui->m_reportltermsTable->setHorizontalHeaderLabels(salesHeaders);
    ui->m_reportltermsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->m_reportltermsTable->horizontalHeader()->setStretchLastSection(true);

    // 实收金额输入框：默认 QDoubleSpinBox 最大只到 99.99，需要放大
    ui->m_paidSpin->setMaximum(999999.99);
    ui->m_paidSpin->setDecimals(2);
}

// ============================================================
//  数据刷新方法
// ============================================================

void MainWindow::updateCashierProductTable(const QList<ProductInfo> &list)
{
    QTableWidget *t = ui->m_cashierProductTable;
    t->setRowCount(0);
    for (const ProductInfo &p : list) {
        int r = t->rowCount();
        t->insertRow(r);
        t->setItem(r, 0, new QTableWidgetItem(p.id));
        t->setItem(r, 1, new QTableWidgetItem(p.name));
        t->setItem(r, 2, new QTableWidgetItem(p.category));
        t->setItem(r, 3, new QTableWidgetItem(QString::number(p.price, 'f', 2)));
        t->setItem(r, 4, new QTableWidgetItem(QString::number(p.stock)));
        t->setItem(r, 5, new QTableWidgetItem(p.condition));
    }
}

void MainWindow::updateProductTable(const QList<ProductInfo> &list)
{
    QTableWidget *t = ui->m_productTable;
    t->setRowCount(0);
    for (const ProductInfo &p : list) {
        int r = t->rowCount();
        t->insertRow(r);
        t->setItem(r, 0, new QTableWidgetItem(p.id));
        t->setItem(r, 1, new QTableWidgetItem(p.name));
        t->setItem(r, 2, new QTableWidgetItem(p.category));
        t->setItem(r, 3, new QTableWidgetItem(QString::number(p.price, 'f', 2)));
        t->setItem(r, 4, new QTableWidgetItem(QString::number(p.stock)));
        t->setItem(r, 5, new QTableWidgetItem(p.condition));
    }
}

void MainWindow::updateCartTable(const QList<CartItem> &items)
{
    QTableWidget *t = ui->m_cartTable;
    t->setRowCount(0);
    for (const CartItem &item : items) {
        int r = t->rowCount();
        t->insertRow(r);
        t->setItem(r, 0, new QTableWidgetItem(item.productId));
        t->setItem(r, 1, new QTableWidgetItem(item.productName));
        t->setItem(r, 2, new QTableWidgetItem(QString::number(item.unitPrice, 'f', 2)));
        t->setItem(r, 3, new QTableWidgetItem(QString::number(item.quantity)));
        t->setItem(r, 4, new QTableWidgetItem(QString::number(item.subtotal, 'f', 2)));
    }
}

void MainWindow::updateCartSummary(int count, double total)
{
    ui->m_cartCountLabel->setText(QString::number(count));
    ui->m_cartTotalLabel->setText(QString::number(total, 'f', 2));
}

void MainWindow::updateOrdersTable(const QList<OrderInfo> &orders)
{
    m_ordersModel->setRowCount(0);
    for (const OrderInfo &o : orders) {
        QList<QStandardItem*> row;
        row.append(new QStandardItem(o.orderId));
        row.append(new QStandardItem(o.dateTime));
        row.append(new QStandardItem(QString::number(o.itemCount)));
        row.append(new QStandardItem(QString::number(o.totalAmount, 'f', 2)));
        row.append(new QStandardItem(o.status));
        m_ordersModel->appendRow(row);
    }
}

void MainWindow::updateSalesTable(const QList<SalesItem> &sales)
{
    QTableWidget *t = ui->m_reportltermsTable;
    t->setRowCount(0);
    for (const SalesItem &s : sales) {
        int r = t->rowCount();
        t->insertRow(r);
        t->setItem(r, 0, new QTableWidgetItem(s.productName));
        t->setItem(r, 1, new QTableWidgetItem(s.productId));
        t->setItem(r, 2, new QTableWidgetItem(QString::number(s.soldCount)));
        t->setItem(r, 3, new QTableWidgetItem(QString::number(s.remainingStock)));
        t->setItem(r, 4, new QTableWidgetItem(""));
    }
}

// ============================================================
//  权限控制
// ============================================================
void MainWindow::setAdminMode(bool adminMode)
{
    Q_UNUSED(adminMode)
    // TODO: 根据角色控制 tab 页可见性
    // Qt 5.15+ 用 setTabVisible，低版本用 setTabEnabled
}
