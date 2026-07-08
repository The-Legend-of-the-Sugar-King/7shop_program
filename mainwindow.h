#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

// ============================================================
//  轻量级数据结构 —— 前后端共用，不放任何业务逻辑
// ============================================================

struct ProductInfo {
    QString id;
    QString name;
    QString category;
    double  price = 0;
    int     stock = 0;
    QString condition;   // "上架" / "下架"
};

struct CartItem {
    QString productId;
    QString productName;
    double  unitPrice = 0;
    int     quantity  = 0;
    double  subtotal  = 0;
};

struct OrderInfo {
    QString orderId;
    QString dateTime;
    int     itemCount   = 0;
    double  totalAmount = 0;
    double  paidAmount  = 0;
    double  changeAmount= 0;
    QString status;       // "已完成" / "已挂起"
};

struct SalesItem {
    QString productName;
    QString productId;
    int     soldCount      = 0;
    int     remainingStock = 0;
};

namespace Ui {
class MainWindow;
}

// ============================================================
//  MainWindow —— 纯前端
//  职责：界面展示 + 控件 getter + 数据刷新入口
// ============================================================
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    // ====== 控件 getter —— 外部通过这些拿控件做 connect / 读写 ======

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

private:
    Ui::MainWindow *ui;
    QStandardItemModel *m_ordersModel;   // 订单明细表 model

    void initTables();
};

#endif // MAINWINDOW_H
