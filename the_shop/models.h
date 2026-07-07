#ifndef MODELS_H
#define MODELS_H
//数据结构
#include <QDateTime>//时间和日期
#include <QList>//容器
#include <QString>

struct product//商品信息
{
    QString id;
    QString name;
    QString type;//商品类别
    double price=0.0;
    int number=0;//库存数量
    bool active=1;//商品是否上架
};

struct Cart_item//购物车中的一项
{
    QString productId;
    int quantity = 0;//number
};

struct Sale_item//销售记录中的商品明细
{
    QString productId;
    QString name;
    double unitPrice = 0.0;//当时的价格
    int quantity = 0;//number
    double subtotal = 0.0;//subtotal = unitPrice * quantity;
};

struct Sale//表示一次完整结账
{
    QString id;
    QDateTime time;
    QList<Sale_item> items;
    double total = 0.0;
    double paid = 0.0;
    double change = 0.0;//找零金额
};

struct SuspendedCart//表示暂时保存但还没有结账的购物车
{
    QString id;//挂单号
    QDateTime time;
    QList<Cart_item> items;
};































#endif // MODELS_H
