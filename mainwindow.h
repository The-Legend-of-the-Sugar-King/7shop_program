#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "data_struct.h"
#include <QMainWindow>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QStandardItemModel>
#include <QPoint>

// ============================================================
//  轻量级数据结构 —— 前后端共用，不放任何业务逻辑
// ============================================================



namespace Ui
{
class MainWindow;
}

// ============================================================
//  MainWindow —— 纯前端
//  职责：界面展示 + 控件 getter + 前端事件转发 + 数据刷新入口
// ============================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // ====== 控件 getter —— 外部通过这些拿控件做必要读写 ======

    // 顶部栏
    QComboBox  *roleCombo() const;

    // 收银台 - 搜索区
    QLineEdit      *productCodeEdit() const;
    QPushButton    *searchByCodeButton() const;
    QLineEdit      *cashierSearchEdit() const;
    QPushButton    *cashierSearchButton() const;
    QSpinBox       *quantitySpin() const;

    // 收银台 - 表格
    QTableWidget   *cashierProductTable() const;
    QTableWidget   *cartTable() const;

    // 收银台 - 汇总
    QLabel         *cartCountLabel() const;
    QLabel         *cartTotalLabel() const;
    QDoubleSpinBox *paidSpin() const;
    QComboBox      *suspendedCombo() const;

    // 收银台 - 按钮
    QPushButton    *addCartButton() const;        // 加入购物车
    QPushButton    *checkoutButton() const;       // 收钱

    // 商品管理
    QTableWidget   *productTable() const;
    QLineEdit      *idEdit() const;
    QPushButton    *searchByIdButton() const;
    QLineEdit      *nameEdit() const;
    QPushButton    *searchByNameButton() const;
    QLineEdit      *categoryEdit() const;
    QPushButton    *searchByCategoryButton() const;
    QDoubleSpinBox *priceSpin() const;
    QSpinBox       *stockSpin() const;
    QComboBox      *activeCombo() const;

    // 销售统计
    QTableView     *reportOrdersTable() const;
    QTableWidget   *reportItemsTable() const;

    // ====== 数据刷新入口 ======

    void updateCashierProductTable(const QList<ProductInfo> &list);
    void updateProductTable(const QList<ProductInfo> &list);
    void updateCartTable(const QList<CartItem> &items);
    void updateCartSummary(int count, double total);

    // 销售统计 —— 收钱后同步到这里
    void updateOrdersTable(const QList<OrderInfo> &orders);
    void updateSalesTable(const QList<SalesItem> &sales);

    // ====== 权限控制 ======
    void setAdminMode(bool adminMode);

signals:
    // 收银台
    void searchByCodeRequested(const QString &code);
    void cashierSearchRequested(const QString &keyword);
    void addToCartRequested(const QString &code, int quantity);
    void removeCartItemRequested(int row);
    void checkoutRequested(double paid);

    // 商品管理
    void searchProductByIdRequested(const QString &id);
    void searchProductByNameRequested(const QString &name);
    void searchProductByCategoryRequested(const QString &category);

    // 权限
    void roleChangedRequested(bool adminMode);

private:
    Ui::MainWindow *ui;
    QStandardItemModel *m_ordersModel;   // 订单明细表 model

    void initTables();
    void connectUiSignals();
    void showCashierProductMenu(const QPoint &pos);
    void showCartMenu(const QPoint &pos);
};

#endif // MAINWINDOW_H
