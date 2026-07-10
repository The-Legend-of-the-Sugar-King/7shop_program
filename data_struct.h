#include <QString>
#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H
struct ProductInfo
{
    QString id;
    QString name;
    QString category;
    double  price = 0;
    int     stock = 0;
    QString condition;   // "上架" / "下架"
};

struct CartItem
{
    QString productId;
    QString productName;
    double  unitPrice = 0;
    int     quantity  = 0;
    double  subtotal  = 0;
};

struct OrderInfo
{
    QString orderId;
    QString dateTime;
    int     itemCount   = 0;
    double  totalAmount = 0;
    double  paidAmount  = 0;
    double  changeAmount= 0;
    QString status;       // "已完成" / "已挂起"
};

struct SalesItem
{
    QString productName;
    QString productId;
    int     soldCount      = 0;
    int     remainingStock = 0;
};
#endif // DATA_STRUCT_H
