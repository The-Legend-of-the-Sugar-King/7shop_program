#include "data_persist.h"
#include "data_struct.h"
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QTextStream>
data_persist::data_persist()
    : data_persist(QDir(QCoreApplication::applicationDirPath()).filePath("data"))
{
}

data_persist::data_persist(QString path)
    : m_file_path(path)
{
    QDir dir(m_file_path);
    m_products_path = dir.filePath("products.txt");
    m_order_path = dir.filePath("orders.txt");
    m_sale_path = dir.filePath("sales.txt");
}

void data_persist::warning(QString *error,QString &message)
{
    if(error!=nullptr)
    {
        *error=message;
    }
}

bool data_persist::ensureDataDir(QString *error )
{
    QDir file_path;
    bool issuccess=file_path.mkpath(this->m_file_path);
    if(!issuccess)
    {
        QString message="文件未能成功创建，请重新尝试";
        warning(error,message);
        return false;
    }

    else
    {
        // QString message="文件成功创建";
        // error=&message;
        return true;
    }


}
bool data_persist::loadAll(QList<ProductInfo> *products,QList<OrderInfo> *orders, QMap<QString, SalesItem> *sales,QString *error)//从硬盘加载到内存:总的加载
{
    bool is_open=ensureDataDir(error);
    if(!is_open)
    {
        // QString error_message="文件夹未能成功打开，加载失败";
        // warning(error,error_message);
        return false;
    }
    if((products==nullptr)||(orders==nullptr)||(sales==nullptr))
    {
        QString error_message="loadAll函数关键参数缺少，无法进行加载";
        warning(error,error_message);
        return false;
    }
//////////////////增加原因，防止加载中出现错误，导致出现不正确数据
    QList<ProductInfo> loading_products;
    QList<OrderInfo> loading_orders;
    QMap<QString, SalesItem> loading_sales;

    if(!loadProducts(&loading_products,error))
    {
        return false;
    }
    if(!loadOrders(&loading_orders,error))
    {
        return false;
    }
    if(!loadSales(&loading_sales,error))
    {
        return false;
    }

    *products=loading_products;
    *orders=loading_orders;
    *sales=loading_sales;


    return true;
}

//具体的加载函数
// id|name|category|price|stock|condition
//  eg       P001|可口可乐|饮料|3.50|100|上架
bool data_persist:: loadProducts(QList<ProductInfo> *products, QString *error)//加载商品表
{
     QFile file(get_m_products_path());
    if(products==nullptr)
     {
         QString message="loadProducts缺少关键参数，无法加载";
        warning(error,message);
         return false;
    }
    if (!ensureDataDir(error))
    {
        return false;
    }
    if(!file.exists())
    {
        products->clear();//第一次打开默认为空文件
        return true;
    }
    if(file.open(QIODevice::ReadOnly|QIODevice::Text)==false)
    {
        QString message="文件打开失败，无法加载";
        warning(error,message);
        return false;
    }
/////////////////////////
    QList<ProductInfo> res_product;
    QTextStream input_from_text(&file);
    int inline_count=0;

    while(!input_from_text.atEnd())
    {
        QString current_line=input_from_text.readLine().trimmed();//细节去掉空格
        inline_count++;
        if(current_line.isEmpty()||current_line.startsWith("#")||current_line.startsWith("|")||current_line.startsWith("id|"))
        {
            continue;
        }

        // if(current_line.startsWith("id|"))
        // {
        //     continue;
        // }
        QStringList parts=current_line.split("|");

        if(parts.size()!=6)
        {
            QString message ="分割的格式不正确";
            warning(error,message);
            return false;
        }

        ProductInfo p1;
        p1.id=parts[0];
        p1.name=parts[1];
        p1.category=parts[2];
        p1.price=parts[3].toDouble();
        p1.stock=parts[4].toInt();
        p1.condition=parts[5];

        if (p1.id.isEmpty())
        {
            QString message = "商品文件第 " + QString::number(inline_count) + " 行商品编号为空";
            warning(error, message);
            return false;
        }

        if (p1.name.isEmpty())
        {
            QString message = "商品文件第 " + QString::number(inline_count) + " 行商品名称为空";
            warning(error, message);
            return false;
        }

        if (p1.category.isEmpty())
        {
            p1.category = "未分类";
        }

        if ((p1.price < 0))
        {
            QString message = "商品文件第 " + QString::number(inline_count) + " 行价格错误";
            warning(error, message);
            return false;
        }

        if ((p1.stock < 0))
        {
            QString message = "商品文件第 " + QString::number(inline_count) + " 行库存错误";
            warning(error, message);
            return false;
        }

        if (p1.condition.isEmpty())
        {
            p1.condition = "上架";
        }

        if ((p1.condition != "上架") && (p1.condition != "下架"))
        {
            QString message = "商品文件第 " + QString::number(inline_count) + " 行状态错误";
            warning(error, message);
            return false;
        }


        res_product.push_back(p1);

    }
    *products=res_product;
    return true;
}
//订单号 | 时间 | 件数 | 应收金额 | 实收金额 | 找零金额 | 状态
bool data_persist::loadOrders(QList<OrderInfo> *orders, QString *error)//加载订单表
{
    QFile file(get_m_order_path());
    if(orders==nullptr)
    {
        QString message="loadorders缺少关键参数，无法加载";
        warning(error,message);
        return false;
    }
    if (!ensureDataDir(error))
    {
        return false;
    }
    if(!file.exists())
    {
        orders->clear();//第一次打开默认为空文件
        return true;
    }
    if(file.open(QIODevice::ReadOnly|QIODevice::Text)==false)
    {
        QString message="文件打开失败，无法加载";
        warning(error,message);
        return false;
    }
    /////////////////////////
    QList<OrderInfo> res_orders;
    QTextStream input_from_text(&file);
    int inline_count=0;

    while(!input_from_text.atEnd())
    {
        QString current_line=input_from_text.readLine().trimmed();//细节去掉空格
        inline_count++;
        if(current_line.isEmpty()||current_line.startsWith("#")||current_line.startsWith("|")||current_line.startsWith("orderId|"))
        {
            continue;
        }

        // if(current_line.startsWith("id|"))
        // {
        //     continue;
        // }
        QStringList parts=current_line.split("|");

        if(parts.size()!=7)
        {
            QString message ="分割的格式不正确";
            warning(error,message);
            return false;
        }

        OrderInfo o1;
        o1.orderId=parts[0].trimmed();
        o1.dateTime=parts[1].trimmed();
        o1.itemCount=parts[2].trimmed().toInt();
        o1.totalAmount=parts[3].trimmed().trimmed().toDouble();
        o1.paidAmount=parts[4].trimmed().toDouble();
        o1.changeAmount=parts[5].trimmed().toDouble();
        o1.status=parts[6].trimmed();
        res_orders.push_back(o1);

    }
    *orders=res_orders;
    return true;
}
//productName|productId|soldCount|remainingStock
bool data_persist::loadSales(QMap<QString, SalesItem> *sales, QString *error)//加载销售但
{
    QFile file(get_m_sale_path());

    if(sales==nullptr)
    {
        QString message="loadSales缺少关键参数，无法加载";
        warning(error,message);
        return false;
    }

    if (!ensureDataDir(error))
    {
        return false;
    }

    if(!file.exists())
    {
        sales->clear();//第一次打开默认为空文件
        return true;
    }

    if(file.open(QIODevice::ReadOnly|QIODevice::Text)==false)
    {
        QString message="文件打开失败，无法加载";
        warning(error,message);
        return false;
    }

    QMap<QString, SalesItem> res_sales;
    QTextStream input_from_text(&file);
    int inline_count=0;

    while(!input_from_text.atEnd())
    {
        QString current_line=input_from_text.readLine().trimmed();
        inline_count++;

        if(current_line.isEmpty() ||
            current_line.startsWith("#") ||
            current_line.startsWith("|") ||
            current_line.startsWith("productName|"))
        {
            continue;
        }

        QStringList parts=current_line.split("|");

        if(parts.size()!=4)
        {
            QString message="销售文件第 "+QString::number(inline_count)+" 行分割格式不正确";
            warning(error,message);
            return false;
        }

        bool soldCountOk=false;
        bool remainingStockOk=false;

        SalesItem s1;
        s1.productName=parts[0].trimmed();
        s1.productId=parts[1].trimmed();
        s1.soldCount=parts[2].trimmed().toInt(&soldCountOk);
        s1.remainingStock=parts[3].trimmed().toInt(&remainingStockOk);

        if(s1.productName.isEmpty())
        {
            QString message="销售文件第 "+QString::number(inline_count)+" 行商品名称为空";
            warning(error,message);
            return false;
        }

        if(s1.productId.isEmpty())
        {
            QString message="销售文件第 "+QString::number(inline_count)+" 行商品编号为空";
            warning(error,message);
            return false;
        }

        if(!soldCountOk || s1.soldCount<0)
        {
            QString message="销售文件第 "+QString::number(inline_count)+" 行已售数量错误";
            warning(error,message);
            return false;
        }

        if(!remainingStockOk || s1.remainingStock<0)
        {
            QString message="销售文件第 "+QString::number(inline_count)+" 行剩余库存错误";
            warning(error,message);
            return false;
        }

        res_sales[s1.productId]=s1;
    }

    *sales=res_sales;
    return true;
}

bool data_persist::saveAll(QList<ProductInfo> &products,QList<OrderInfo> &orders,QMap<QString, SalesItem> &sales,QString *error)
{
    if(!ensureDataDir(error))
    {
        return false;
    }

    if(!saveProducts(products,error))
    {
        return false;
    }

    if(!saveOrders(orders,error))
    {
        return false;
    }

    if(!saveSales(sales,error))
    {
        return false;
    }

    return true;
}
//子函数
// bool saveProducts(QList<ProductInfo> &products, QString *error = nullptr);//保存商品表
// bool saveOrders( QList<OrderInfo> &orders, QString *error = nullptr);//保存订单表
// bool saveSales(QMap<QString, SalesItem> &sales, QString *error = nullptr);//保存销售单
bool data_persist::saveProducts(QList<ProductInfo> &products, QString *error)//保存商品表
{
    if(!ensureDataDir(error))
    {
        return false;
    }

    QFile productFile(get_m_products_path());
    if(productFile.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate)==false)
    {
        QString message="商品文件打开失败，无法保存";
        warning(error,message);
        return false;
    }

    QTextStream productOut(&productFile);
    productOut<<"# id|name|category|price|stock|condition\n";
    productOut<<"id|name|category|price|stock|condition\n";

    for(int i=0;i<products.size();i++)
    {
        ProductInfo p=products[i];
        productOut<<p.id<<"|"
                   <<p.name<<"|"
                   <<p.category<<"|"
                   <<QString::number(p.price,'f',2)<<"|"
                   <<p.stock<<"|"
                   <<p.condition
                   <<"\n";
    }

    productFile.close();
    return true;
}

bool data_persist::saveOrders(QList<OrderInfo> &orders, QString *error)//保存订单表
{
    if(!ensureDataDir(error))
    {
        return false;
    }

    QFile orderFile(get_m_order_path());
    if(orderFile.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate)==false)
    {
        QString message="订单文件打开失败，无法保存";
        warning(error,message);
        return false;
    }

    QTextStream orderOut(&orderFile);
    orderOut<<"# orderId|dateTime|itemCount|totalAmount|paidAmount|changeAmount|status\n";
    orderOut<<"orderId|dateTime|itemCount|totalAmount|paidAmount|changeAmount|status\n";

    for(int i=0;i<orders.size();i++)
    {
        OrderInfo o=orders[i];
        orderOut<<o.orderId<<"|"
                 <<o.dateTime<<"|"
                 <<o.itemCount<<"|"
                 <<QString::number(o.totalAmount,'f',2)<<"|"
                 <<QString::number(o.paidAmount,'f',2)<<"|"
                 <<QString::number(o.changeAmount,'f',2)<<"|"
                 <<o.status
                 <<"\n";
    }

    orderFile.close();
    return true;
}

bool data_persist::saveSales(QMap<QString, SalesItem> &sales, QString *error)//保存销售单
{
    if(!ensureDataDir(error))
    {
        return false;
    }

    QFile saleFile(get_m_sale_path());
    if(saleFile.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Truncate)==false)
    {
        QString message="销售文件打开失败，无法保存";
        warning(error,message);
        return false;
    }

    QTextStream saleOut(&saleFile);
    saleOut<<"# productName|productId|soldCount|remainingStock\n";
    saleOut<<"productName|productId|soldCount|remainingStock\n";

    QMap<QString, SalesItem>::iterator it;
    for(it=sales.begin();it!=sales.end();it++)
    {
        SalesItem s=it.value();
        saleOut<<s.productName<<"|"
                <<s.productId<<"|"
                <<s.soldCount<<"|"
                <<s.remainingStock
                <<"\n";
    }

    saleFile.close();
    return true;
}






//一些接口
QString data_persist::get_file_path()
{
    return this->m_file_path;
}

QString data_persist::get_m_products_path()
{
    return this->m_products_path;
}
QString data_persist::get_m_order_path()
{
    return this->m_order_path;
}
QString data_persist::get_m_sale_path()
{
    return this->m_sale_path;
}

























