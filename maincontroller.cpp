#include "maincontroller.h"
#include <QMessageBox>
#include <QMenu>
#include <QDateTime>

MainController::MainController(MainWindow *view, QObject *parent)
    : QObject(parent)
    , m_view(view)
{
    connectSignals();
    loadDemoProducts();
}

MainController::~MainController()
{
}

// ============================================================
//  信号槽连接
// ============================================================
void MainController::connectSignals()
{
    // ---- 收银台 ----
    connect(m_view->searchByCodeButton(), &QPushButton::clicked,
            this, &MainController::onSearchByCode);
    connect(m_view->cashierSearchButton(), &QPushButton::clicked,
            this, &MainController::onSearchProduct);
    connect(m_view->addCartButton(), &QPushButton::clicked,
            this, &MainController::onAddToCart);
    connect(m_view->cashierProductTable(), &QTableWidget::customContextMenuRequested,
            this, &MainController::onCashierProductRightClick);  // 右键加入购物车
    connect(m_view->cartTable(), &QTableWidget::customContextMenuRequested,
            this, &MainController::onRemoveFromCart);             // 右键移出购物车
    connect(m_view->checkoutButton(), &QPushButton::clicked,
            this, &MainController::onCheckout);                   // ★ 收钱

    // ---- 商品管理 ----
    connect(m_view->searchByIdButton(), &QPushButton::clicked,
            this, &MainController::onSearchProductById);
    connect(m_view->searchByNameButton(), &QPushButton::clicked,
            this, &MainController::onSearchProductByName);
    connect(m_view->searchByCategoryButton(), &QPushButton::clicked,
            this, &MainController::onSearchProductByCategory);

    // ---- 权限 ----
    connect(m_view->roleCombo(), SIGNAL(currentIndexChanged(int)),
            this, SLOT(onRoleChanged(int)));
}

// ============================================================
//  演示数据（后端接入后删除整段）
// ============================================================
void MainController::loadDemoProducts()
{
    m_products.clear();

    ProductInfo p;
    p.id = "P001"; p.name = "可口可乐";     p.category = "饮料";  p.price = 3.50;  p.stock = 100; p.condition = "上架"; m_products.append(p);
    p.id = "P002"; p.name = "农夫山泉";     p.category = "饮料";  p.price = 2.00;  p.stock = 200; p.condition = "上架"; m_products.append(p);
    p.id = "P003"; p.name = "康师傅方便面";  p.category = "食品";  p.price = 4.50;  p.stock = 80;  p.condition = "上架"; m_products.append(p);
    p.id = "P004"; p.name = "奥利奥饼干";   p.category = "食品";  p.price = 9.90;  p.stock = 50;  p.condition = "上架"; m_products.append(p);
    p.id = "P005"; p.name = "高露洁牙膏";   p.category = "日用品"; p.price = 12.90; p.stock = 60;  p.condition = "上架"; m_products.append(p);
    p.id = "P006"; p.name = "心相印纸巾";   p.category = "日用品"; p.price = 5.90;  p.stock = 120; p.condition = "上架"; m_products.append(p);
    p.id = "P007"; p.name = "青岛啤酒";     p.category = "酒类";  p.price = 6.00;  p.stock = 40;  p.condition = "上架"; m_products.append(p);
    p.id = "P008"; p.name = "金龙鱼调和油";  p.category = "食品";  p.price = 59.90; p.stock = 30;  p.condition = "上架"; m_products.append(p);
}

// ============================================================
//  初始化
// ============================================================
void MainController::init()
{
    refreshCashierProducts(m_products);
    refreshProducts(m_products);
    refreshSalesDisplay();
}

// ============================================================
//  收银台：按编号搜索
// ============================================================
void MainController::onSearchByCode()
{
    QString code = m_view->productCodeEdit()->text().trimmed();
    QList<ProductInfo> result;
    if (code.isEmpty()) {
        result = m_products;
    } else {
        for (const ProductInfo &p : m_products) {
            if (p.id == code) { result.append(p); break; }
        }
    }
    refreshCashierProducts(result);
}

// ============================================================
//  收银台：按名称搜索
// ============================================================
void MainController::onSearchProduct()
{
    QString kw = m_view->cashierSearchEdit()->text().trimmed();
    QList<ProductInfo> result;
    if (kw.isEmpty()) {
        result = m_products;
    } else {
        for (const ProductInfo &p : m_products) {
            if (p.name.contains(kw, Qt::CaseInsensitive))
                result.append(p);
        }
    }
    refreshCashierProducts(result);
}

// ============================================================
//  收银台：加入购物车（按钮触发）
// ============================================================
void MainController::onAddToCart()
{
    QString code = m_view->productCodeEdit()->text().trimmed();
    int qty = m_view->quantitySpin()->value();
    addProductToCart(code, qty);
}

// ============================================================
//  收银台：右键商品 → 加入购物车
// ============================================================
void MainController::onCashierProductRightClick(const QPoint &pos)
{
    QTableWidget *table = m_view->cashierProductTable();
    QTableWidgetItem *item = table->itemAt(pos);
    if (!item) return;

    int row = item->row();
    QTableWidgetItem *codeItem = table->item(row, 0);
    if (!codeItem) return;

    QString code = codeItem->text();
    int qty = m_view->quantitySpin()->value();

    QMenu menu;
    QAction *addAction = menu.addAction(QString("加入购物车（%1 件）").arg(qty));
    QAction *chosen = menu.exec(table->viewport()->mapToGlobal(pos));

    if (chosen == addAction) {
        addProductToCart(code, qty);
    }
}

// ============================================================
//  加入购物车核心逻辑
// ============================================================
void MainController::addProductToCart(const QString &code, int qty)
{
    if (code.isEmpty()) {
        QMessageBox::warning(m_view, "提示", "请先选择商品");
        return;
    }
    if (qty <= 0) {
        QMessageBox::warning(m_view, "提示", "数量必须大于0");
        return;
    }

    // 查找商品
    ProductInfo *target = nullptr;
    for (int i = 0; i < m_products.size(); ++i) {
        if (m_products[i].id == code) { target = &m_products[i]; break; }
    }
    if (!target) {
        QMessageBox::warning(m_view, "提示", "未找到该商品");
        return;
    }
    if (target->stock < qty) {
        QMessageBox::warning(m_view, "库存不足",
            QString("商品 \"%1\" 库存仅剩 %2 件").arg(target->name).arg(target->stock));
        return;
    }

    // 购物车已有则合并
    bool found = false;
    for (int i = 0; i < m_cartItems.size(); ++i) {
        if (m_cartItems[i].productId == target->id) {
            m_cartItems[i].quantity += qty;
            m_cartItems[i].subtotal = m_cartItems[i].unitPrice * m_cartItems[i].quantity;
            found = true;
            break;
        }
    }
    if (!found) {
        CartItem item;
        item.productId   = target->id;
        item.productName = target->name;
        item.unitPrice   = target->price;
        item.quantity    = qty;
        item.subtotal    = target->price * qty;
        m_cartItems.append(item);
    }

    refreshCartDisplay();
}

// ============================================================
//  购物车：右键移出
// ============================================================
void MainController::onRemoveFromCart(const QPoint &pos)
{
    QTableWidget *cart = m_view->cartTable();
    QTableWidgetItem *item = cart->itemAt(pos);
    if (!item) return;

    int row = item->row();
    if (row < 0 || row >= m_cartItems.size()) return;

    QMenu menu;
    QAction *removeAction = menu.addAction("移出购物车");
    QAction *chosen = menu.exec(cart->viewport()->mapToGlobal(pos));

    if (chosen == removeAction) {
        m_cartItems.removeAt(row);
        refreshCartDisplay();
    }
}

// ============================================================
//  ★ 收钱 —— 结算 + 同步销售统计表
// ============================================================
void MainController::onCheckout()
{
    if (m_cartItems.isEmpty()) {
        QMessageBox::warning(m_view, "提示", "购物车为空，无法收款");
        return;
    }

    double paid = m_view->paidSpin()->value();

    // 计算应收
    double total = 0;
    int totalQty = 0;
    for (const CartItem &item : m_cartItems) {
        total    += item.subtotal;
        totalQty += item.quantity;
    }

    if (paid < total) {
        QMessageBox::warning(m_view, "金额不足",
            QString("应收 %1 元，实收 %2 元，还差 %3 元")
                .arg(total, 0, 'f', 2)
                .arg(paid, 0, 'f', 2)
                .arg(total - paid, 0, 'f', 2));
        return;
    }

    // ---- 1. 创建订单 ----
    OrderInfo order;
    order.orderId    = QString("ORD%1").arg(
        QDateTime::currentDateTime().toString("yyyyMMddhhmmss"));
    order.dateTime   = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    order.itemCount  = totalQty;
    order.totalAmount= total;
    order.paidAmount = paid;
    order.changeAmount = paid - total;
    order.status     = "已完成";

    m_orders.prepend(order);   // 最新的排最前

    // ---- 2. 扣库存 + 累计销量 ----
    for (const CartItem &ci : m_cartItems) {
        // 扣库存
        for (int i = 0; i < m_products.size(); ++i) {
            if (m_products[i].id == ci.productId) {
                m_products[i].stock -= ci.quantity;
                break;
            }
        }

        // 累计销售统计
        if (m_sales.contains(ci.productId)) {
            m_sales[ci.productId].soldCount += ci.quantity;
        } else {
            SalesItem si;
            si.productId   = ci.productId;
            si.productName = ci.productName;
            si.soldCount   = ci.quantity;
            si.remainingStock = 0;   // 下面更新
            m_sales[ci.productId] = si;
        }
    }

    // 更新销量中的剩余库存
    for (auto it = m_sales.begin(); it != m_sales.end(); ++it) {
        for (const ProductInfo &p : m_products) {
            if (p.id == it.key()) {
                it->remainingStock = p.stock;
                break;
            }
        }
    }

    // ---- 3. 刷新所有显示 ----
    refreshSalesDisplay();
    refreshCashierProducts(m_products);
    refreshProducts(m_products);

    // ---- 4. 清空购物车 ----
    m_cartItems.clear();
    refreshCartDisplay();
    m_view->paidSpin()->setValue(0);

    // ---- 5. 提示 ----
    QMessageBox::information(m_view, "收款成功",
        QString("订单号: %1\n"
                "件数: %2\n"
                "应收: %3 元\n"
                "实收: %4 元\n"
                "找零: %5 元")
            .arg(order.orderId)
            .arg(order.itemCount)
            .arg(order.totalAmount, 0, 'f', 2)
            .arg(order.paidAmount, 0, 'f', 2)
            .arg(order.changeAmount, 0, 'f', 2));
}

// ============================================================
//  商品管理：按编号搜索
// ============================================================
void MainController::onSearchProductById()
{
    QString id = m_view->idEdit()->text().trimmed();
    QList<ProductInfo> result;
    if (id.isEmpty()) {
        result = m_products;
    } else {
        for (const ProductInfo &p : m_products) {
            if (p.id == id) { result.append(p); break; }
        }
    }
    refreshProducts(result);
}

// ============================================================
//  商品管理：按名称搜索
// ============================================================
void MainController::onSearchProductByName()
{
    QString name = m_view->nameEdit()->text().trimmed();
    QList<ProductInfo> result;
    if (name.isEmpty()) {
        result = m_products;
    } else {
        for (const ProductInfo &p : m_products) {
            if (p.name.contains(name, Qt::CaseInsensitive))
                result.append(p);
        }
    }
    refreshProducts(result);
}

// ============================================================
//  商品管理：按类别搜索
// ============================================================
void MainController::onSearchProductByCategory()
{
    QString cat = m_view->categoryEdit()->text().trimmed();
    QList<ProductInfo> result;
    if (cat.isEmpty()) {
        result = m_products;
    } else {
        for (const ProductInfo &p : m_products) {
            if (p.category.contains(cat, Qt::CaseInsensitive))
                result.append(p);
        }
    }
    refreshProducts(result);
}

// ============================================================
//  权限切换
// ============================================================
void MainController::onRoleChanged(int index)
{
    bool isAdmin = (index == 0);
    m_view->setAdminMode(isAdmin);
}

// ============================================================
//  刷新辅助方法
// ============================================================
void MainController::refreshCashierProducts(const QList<ProductInfo> &list)
{
    m_view->updateCashierProductTable(list);
}

void MainController::refreshProducts(const QList<ProductInfo> &list)
{
    m_view->updateProductTable(list);
}

void MainController::refreshCartDisplay()
{
    m_view->updateCartTable(m_cartItems);
    int count = 0;
    double total = 0;
    for (const CartItem &item : m_cartItems) {
        count += item.quantity;
        total += item.subtotal;
    }
    m_view->updateCartSummary(count, total);
}

void MainController::refreshSalesDisplay()
{
    // 订单明细
    m_view->updateOrdersTable(m_orders);

    // 商品销量 —— 显示所有商品（未售出的显示 0）
    QList<SalesItem> salesList;
    for (const ProductInfo &p : m_products) {
        SalesItem si;
        si.productId      = p.id;
        si.productName    = p.name;
        si.remainingStock = p.stock;
        si.soldCount      = m_sales.contains(p.id) ? m_sales[p.id].soldCount : 0;
        salesList.append(si);
    }
    m_view->updateSalesTable(salesList);
}
