#include<QString>
#include<QList>
#include "data_struct.h"
#include <QMap>

#ifndef DATA_PERSIST_H
#define DATA_PERSIST_H
//这个类专门进行数据的持久化
class data_persist
{
public:
    //构造方法
    data_persist(QString m_file_path);
    data_persist();
    //商品的保存
    bool loadAll(QList<ProductInfo> *products,QList<OrderInfo> *orders, QMap<QString, SalesItem> *sales,QString *error = nullptr);//从硬盘加载到内存
    bool saveAll(QList<ProductInfo> &products,QList<OrderInfo> &orders,QMap<QString, SalesItem> &sales,QString *error = nullptr);//从内存加载到硬盘
    bool loadProducts(QList<ProductInfo> *products, QString *error = nullptr);//加载商品表
    bool saveProducts(QList<ProductInfo> &products, QString *error = nullptr);//保存商品表
    bool loadOrders(QList<OrderInfo> *orders, QString *error = nullptr);//加载订单表
    bool saveOrders( QList<OrderInfo> &orders, QString *error = nullptr);//保存订单表
    bool loadSales(QMap<QString, SalesItem> *sales, QString *error = nullptr);//加载销售但
    bool saveSales(QMap<QString, SalesItem> &sales, QString *error = nullptr);//保存销售单

    QString get_file_path();
    QString get_m_products_path();
    QString get_m_order_path();
    QString get_m_sale_path();
private:
    QString m_file_path;
    QString m_products_path;
    QString m_order_path;
    QString m_sale_path;
    //辅助函数
//报错函数
    void warning(QString *error,QString &message);
    bool ensureDataDir(QString *error = nullptr) ;

};

#endif // DATA_PERSIST_H
