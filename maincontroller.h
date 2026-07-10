#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QList>
#include <QMap>
#include "mainwindow.h"
#include "data_struct.h"
#include "data_persist.h"
// ============================================================
//  MainController —— 连接层
//  职责：
//    1. connect MainWindow 的业务信号到控制器槽函数
//    2. 管理临时数据（购物车、订单、销售统计）
//    3. 收钱流程 → 同步到销售统计表
//
//  后端接入方式：
//    把所有 m_xxxData 的读写替换为后端 API 调用即可，
//    数据结构 ProductInfo / CartItem / OrderInfo / SalesItem
//    前后端共用，不需要改任何头文件。
// ============================================================
class MainController : public QObject
{
    Q_OBJECT

public:
    explicit MainController(MainWindow *view, QObject *parent = nullptr);
    ~MainController();

    void init();   // 加载初始数据

private slots:
    // ---- 收银台 ----
    void onSearchByCode(const QString &code);
    void onSearchProduct(const QString &keyword);
    void onAddToCart(const QString &code, int qty);
    void onRemoveFromCart(int row);
    void onCheckout(double paid);                        // ★ 收钱 → 同步销售表

    // ---- 商品管理 ----
    void onSearchProductById(const QString &id);
    void onSearchProductByName(const QString &name);
    void onSearchProductByCategory(const QString &category);

    // ---- 权限 ----
    void onRoleChanged(bool adminMode);

private:
    void connectSignals();
    void loadDemoProducts();        // 演示用初始数据（后端接入后删除）
    void addProductToCart(const QString &code, int qty);  // 加入购物车核心逻辑

    void refreshCashierProducts(const QList<ProductInfo> &list);
    void refreshProducts(const QList<ProductInfo> &list);
    void refreshCartDisplay();
    void refreshSalesDisplay();     // 刷新销售统计两个表

    MainWindow *m_view;

    // ---- 前端维护的临时数据（后端接入后可移到后端） ----
    QList<ProductInfo> m_products;       // 商品列表
    QList<CartItem>    m_cartItems;      // 购物车
    QList<OrderInfo>   m_orders;         // 订单历史
    QMap<QString, SalesItem> m_sales;    // key=productId, 商品销量累计
    data_persist m_dataPersist;          // 文本文件持久化
};

#endif // MAINCONTROLLER_H
