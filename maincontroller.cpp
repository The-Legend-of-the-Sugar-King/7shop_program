#include "data_struct.h"
#include "maincontroller.h"
#include <QMessageBox>
#include <QDateTime>

MainController::MainController(MainWindow *view, QObject *parent)
    : QObject(parent)
    , m_view(view)
{
    connectSignals();
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
    connect(m_view, &MainWindow::searchByCodeRequested,
            this, &MainController::onSearchByCode);
    connect(m_view, &MainWindow::cashierSearchRequested,
            this, &MainController::onSearchProduct);
    connect(m_view, &MainWindow::addToCartRequested,
            this, &MainController::onAddToCart);
    connect(m_view, &MainWindow::removeCartItemRequested,
            this, &MainController::onRemoveFromCart);
    connect(m_view, &MainWindow::checkoutRequested,
            this, &MainController::onCheckout);                   // ★ 收钱

    // ---- 商品管理 ----
    connect(m_view, &MainWindow::searchProductByIdRequested,
            this, &MainController::onSearchProductById);
    connect(m_view, &MainWindow::searchProductByNameRequested,
            this, &MainController::onSearchProductByName);
    connect(m_view, &MainWindow::searchProductByCategoryRequested,
            this, &MainController::onSearchProductByCategory);

    // ---- 权限 ----
    connect(m_view, &MainWindow::roleChangedRequested,
            this, &MainController::onRoleChanged);
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
    QString error;
    if(!m_dataPersist.loadAll(&m_products,&m_orders,&m_sales,&error))
    {
        QMessageBox::warning(m_view, "数据加载失败", error);
        loadDemoProducts();
    }

    if(m_products.isEmpty())
    {
        loadDemoProducts();
        QString saveError;
        if(!m_dataPersist.saveAll(m_products,m_orders,m_sales,&saveError))
        {
            QMessageBox::warning(m_view, "初始数据保存失败", saveError);
        }
    }

    refreshCashierProducts(m_products);
    refreshProducts(m_products);
    refreshSalesDisplay();
}

// ============================================================
//  收银台：按编号搜索
// ============================================================
void MainController::onSearchByCode(const QString &codeText)
{
    QString code = codeText.trimmed();
    QList<ProductInfo> result;
    if (code.isEmpty())
    {
        result = m_products;
    } else
    {
        for (int i=0;i<m_products.size();i++)
        {
            if (m_products[i].id == code)
            {
                result.append(m_products[i]);
                break;
            }
        }
    }
    refreshCashierProducts(result);
}

// ============================================================
//  收银台：按名称搜索
// ============================================================
void MainController::onSearchProduct(const QString &Product_name)
{
    QString pn = Product_name.trimmed();
    QList<ProductInfo> result;
    if (pn.isEmpty())
    {
        result = m_products;
    } else
    {
        for (int i=0;i<m_products.size();i++)
        {
            if (m_products[i].name.contains(pn, Qt::CaseInsensitive))
                result.push_back(m_products[i]);
        }
    }
    refreshCashierProducts(result);
}

// ============================================================
//  收银台：加入购物车（按钮触发）
// ============================================================
void MainController::onAddToCart(const QString &codeText, int qty)
{
    QString code = codeText.trimmed();
    addProductToCart(code, qty);
}

// ============================================================
//  加入购物车核心逻辑
// ============================================================
void MainController::addProductToCart(const QString &code, int qty)
{
    if (code.isEmpty())
    {
        QMessageBox::warning(m_view, "提示", "请先选择商品");
        return;
    }
    if (qty <= 0)
    {
        QMessageBox::warning(m_view, "提示", "数量必须大于0");
        return;
    }

    // 查找商品
    ProductInfo *target = nullptr;
    for (int i = 0; i < m_products.size(); i++)
    {
        if (m_products[i].id == code)
        {
            target = &m_products[i];
            break;
        }
    }
    if (target==nullptr)
    {
        QMessageBox::warning(m_view, "提示", "未找到该商品");
        return;
    }
    if (target->stock < qty)
    {

        QString s1=QString::number(target->stock);

        QString message="商品"+target->name+"库存仅剩"+s1+"件";
        QMessageBox::warning(m_view, "库存不足",message);
        return;
    }

    // 购物车已有则合并
    bool found = false;
    for (int i = 0; i < m_cartItems.size(); i++)
    {
        if (m_cartItems[i].productId == target->id)
        {
            m_cartItems[i].quantity += qty;
            m_cartItems[i].subtotal = m_cartItems[i].unitPrice * m_cartItems[i].quantity;
            found = true;
            break;
        }
    }
    if (!found)
    {
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
void MainController::onRemoveFromCart(int row)
{
    if (row < 0 || row >= m_cartItems.size()) return;

    m_cartItems.removeAt(row);
    refreshCartDisplay();
}

// ============================================================
//  ★ 收钱 —— 结算 + 同步销售统计表
// ============================================================
void MainController::onCheckout(double paid)
{
    if (m_cartItems.isEmpty())
    {
        QMessageBox::warning(m_view, "提示", "购物车为空，无法收款");
        return;
    }

    // 计算应收
    double total = 0;
    int totalQty = 0;
    for (const CartItem &item : m_cartItems)
    {
        total    += item.subtotal;
        totalQty += item.quantity;
    }

    if (paid < total)
    {
        QString s1,s2,s3,message;
        s1=QString::number(total);
        s2=QString::number(paid);
        s3=QString::number(total - paid);
        message="应收"+s1+"元，实收 "+s2+"元，还差 "+s3+" 元";
        QMessageBox::warning(m_view, "金额不足",message);
        return;
    }

    // ---- 1. 创建订单 ----
    OrderInfo order;
     QString res="ORD"+QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
    order.orderId    = res;
    order.dateTime   = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    order.itemCount  = totalQty;
    order.totalAmount= total;
    order.paidAmount = paid;
    order.changeAmount = paid - total;
    order.status     = "已完成";

   m_orders.push_front(order);   // 最新的排最前

    // ---- 2. 扣库存 + 累计销量 ----
    for (int i=0;i<m_cartItems.size();i++)
   {
        // 扣库存
        for (int j = 0; j < m_products.size(); j++)
        {
            if (m_products[j].id == m_cartItems[i].productId)
            {
                m_products[j].stock -=m_cartItems[i].quantity;
                break;
            }
        }

        // 累计销售统计
        if (m_sales.contains(m_cartItems[i].productId))
        {
            m_sales[m_cartItems[i].productId].soldCount +=m_cartItems[i].quantity;
        }
        else
        {
            SalesItem si;
            si.productId   = m_cartItems[i].productId;
            si.productName = m_cartItems[i].productName;
            si.soldCount   = m_cartItems[i].quantity;
            si.remainingStock = 0;   // 下面更新
            m_sales[m_cartItems[i].productId] = si;
        }
    }

    // 更新销量中的剩余库存
    for (auto it = m_sales.begin(); it != m_sales.end(); it++)
    {
        for (int i = 0; i < m_products.size(); i++)
        {
            if (m_products[i].id == it.key())
            {
                it->remainingStock = m_products[i].stock;
                break;
            }
        }
    }

    QString saveError;
    if(!m_dataPersist.saveAll(m_products,m_orders,m_sales,&saveError))
    {
        QMessageBox::warning(m_view, "数据保存失败", saveError);
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
    QString message;
    message += "订单号: " + order.orderId + "\n";
    message += "件数: " + QString::number(order.itemCount) + "\n";
    message += "应收: " + QString::number(order.totalAmount, 'f', 2) + " 元\n";
    message += "实收: " + QString::number(order.paidAmount, 'f', 2) + " 元\n";
    message += "找零: " + QString::number(order.changeAmount, 'f', 2) + " 元";

    QMessageBox::information(m_view, "收款成功", message);
}

// ============================================================
//  商品管理：按编号搜索
// ============================================================
void MainController::onSearchProductById(const QString &idText)
{
    QString id = idText.trimmed();
    QList<ProductInfo> result;
    if (id.isEmpty())
    {
        result = m_products;
    }
    else
    {
        for (int i = 0; i < m_products.size(); i++)
        {
            if (m_products[i].id == id)
            {
                result.append(m_products[i]);
                break;
            }
        }
    }
    refreshProducts(result);
}

// ============================================================
//  商品管理：按名称搜索
// ============================================================
void MainController::onSearchProductByName(const QString &nameText)
{
    QString name = nameText.trimmed();
    QList<ProductInfo> result;
    if (name.isEmpty())
    {
        result = m_products;
    }
    else
    {
        for (int i = 0; i < m_products.size(); i++)
        {
            if (m_products[i].name.contains(name, Qt::CaseInsensitive))
            {
                result.push_back(m_products[i]);
            }
        }
    }
    refreshProducts(result);
}

// ============================================================
//  商品管理：按类别搜索
// ============================================================
void MainController::onSearchProductByCategory(const QString &categoryText)
{
    QString cat = categoryText.trimmed();
    QList<ProductInfo> result;
    if (cat.isEmpty())
    {
        result = m_products;
    }
    else
    {
        for (int i = 0; i < m_products.size(); i++)
        {
            if (m_products[i].category.contains(cat, Qt::CaseInsensitive))//忽略大小写
                result.push_back(m_products[i]);
        }
    }
    refreshProducts(result);
}

// ============================================================
//  权限切换
// ============================================================
void MainController::onRoleChanged(bool adminMode)//待完成
{
    m_view->setAdminMode(adminMode);
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
    for (int i = 0; i < m_cartItems.size(); i++)
    {
        count += m_cartItems[i].quantity;
        total += m_cartItems[i].subtotal;
    }
    m_view->updateCartSummary(count, total);
}

void MainController::refreshSalesDisplay()
{
    // 订单明细
    m_view->updateOrdersTable(m_orders);

    // 商品销量 —— 显示所有商品（未售出的显示 0）
    QList<SalesItem> salesList;
    for (int i = 0; i < m_products.size(); i++)
    {
        SalesItem si;
        si.productId      = m_products[i].id;
        si.productName    = m_products[i].name;
        si.remainingStock = m_products[i].stock;
        if (m_sales.contains(m_products[i].id))
        {
            si.soldCount = m_sales[m_products[i].id].soldCount;
        }
        else
        {
            si.soldCount = 0;
        }
        salesList.push_back(si);
    }
    m_view->updateSalesTable(salesList);
}
