#ifndef STORE_H
#define STORE_H
#include <QHash>
#include <QList>
#include <QString>
#include "models.h"//自己写的数据结构

class store//数据保存中心
{
public:
  //构造函数
    store(QString dataDir);//dataDir为数据文件所在目录
    bool load(QString *error = nullptr);//加载数据
    bool save(QString *error = nullptr) const;//上传数据
  //获取数据列表
    const QList<product> &products() const;//商品列表
    const QList<Sale> &sales() const;//销售记录列表
    const QList<SuspendedCart> &suspendedCarts() const;//挂单列表
    const product *findProduct(const QString &id) const;//根据商品 ID 查找商品。
  //商品管理函数
    bool addProduct(product product, QString *error = nullptr);//添加商品
    bool updateProduct(product product, QString *error = nullptr);//更新商品
    bool removeProduct(const QString &id, QString *error = nullptr);//删除商品
   bool setProductActive(const QString &id, bool active, QString *error = nullptr);//设置商品的上架与否
   //购物车相关函数
    bool validateCart(const QList<Cart_item> &cart, QString *error = nullptr) const;
    double cartTotal(const QList<Cart_item> &cart) const;//计算购物车总价
    int cartItemCount(const QList<Cart_item> &cart) const;//计算购物车商品总件数
    //结账相关函数
    bool commitSale(const QList<Cart_item> &cart, double paid, Sale *createdSale, QString *error = nullptr);
    //挂单相关函数
    bool suspendCart(const QList<Cart_item> &cart, SuspendedCart *createdCart, QString *error = nullptr);
    bool takeSuspendedCart(const QString &id, QList<Cart_item> *cart, QString *error = nullptr);

    QString dataDir() const;//返回数据存储的地址
private:
    QList<product> m_products;
    QList<Sale> m_sales;
    QList<SuspendedCart> m_suspendedCarts;
    QString m_dataDir;
    QHash<QString, int> m_productIndex;//商品 ID → 商品在 m_products 中的下标
    //辅助函数
    QString productsPath() const;
    QString salesPath() const;
    QString suspendedPath() const;
    void rebuildIndex();
    void seedDefaultProducts();
    QString nextSaleId() const;
    QString nextSuspendedId() const;
    bool validateProduct(const product &product, bool requireNew, QString *error) const;

};























#endif // STORE_H
