#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QIODevice>
#include <QStringList>
#include <QTextStream>

#include "store.h"

// 这个函数只是为了少写几次 if(error)，不是类成员函数。
// 初学时也可以直接在每个地方写 if(error) *error = "...";
void writeError(QString *error, const QString &message)
{
    if (error != nullptr) {
        *error = message;
    }
}

// 判断一行是不是有效数据行。空行和 # 开头的说明行不处理。
bool isUsefulLine(QString line)
{
    line = line.trimmed();
    if (line.isEmpty()) {
        return false;
    }
    if (line.startsWith("#")) {
        return false;
    }
    return true;
}

store::store(QString dataDir)
{
    m_dataDir = dataDir;
}

bool store::load(QString *error)
{
    QDir dir;
    dir.mkpath(m_dataDir);

    m_products.clear();
    m_sales.clear();
    m_suspendedCarts.clear();

    bool needSave = false;

    // 1. 读取商品文件 products.txt
    // 格式：id|name|type|price|number|active
    if (QFileInfo::exists(productsPath())) {
        QFile file(productsPath());

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            writeError(error, "无法读取商品文件：" + productsPath());
            return false;
        }

        QTextStream in(&file);
        int lineNumber = 0;

        while (!in.atEnd()) {
            QString line = in.readLine();
            lineNumber++;

            if (!isUsefulLine(line)) {
                continue;
            }

            QStringList list = line.split("|");
            if (list.size() != 6) {
                writeError(error, QString("商品文件第 %1 行格式错误。").arg(lineNumber));
                file.close();
                return false;
            }

            bool priceOk = false;
            bool numberOk = false;

            product p;
            p.id = list[0].trimmed();
            p.name = list[1].trimmed();
            p.type = list[2].trimmed();
            p.price = list[3].toDouble(&priceOk);
            p.number = list[4].toInt(&numberOk);
            p.active = list[5].trimmed() != "0";

            if (!priceOk || !numberOk) {
                writeError(error, QString("商品文件第 %1 行价格或库存不是数字。").arg(lineNumber));
                file.close();
                return false;
            }

            if (p.type.isEmpty()) {
                p.type = "未分类";
            }

            m_products.append(p);
        }

        file.close();
    } else {
        seedDefaultProducts();
        needSave = true;
    }

    // 2. 读取销售记录 sales.txt
    // 格式：
    // SALE|id|time|total|paid|change
    // ITEM|productId|name|unitPrice|quantity|subtotal
    // ENDSALE
    if (QFileInfo::exists(salesPath())) {
        QFile file(salesPath());

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            writeError(error, "无法读取销售文件：" + salesPath());
            return false;
        }

        QTextStream in(&file);
        Sale sale;
        bool readingSale = false;
        int lineNumber = 0;

        while (!in.atEnd()) {
            QString line = in.readLine();
            lineNumber++;

            if (!isUsefulLine(line)) {
                continue;
            }

            QStringList list = line.split("|");
            QString first = list.value(0);

            if (first == "SALE") {
                if (list.size() != 6) {
                    writeError(error, QString("销售文件第 %1 行格式错误。").arg(lineNumber));
                    file.close();
                    return false;
                }

                bool totalOk = false;
                bool paidOk = false;
                bool changeOk = false;

                sale = Sale();
                sale.id = list[1].trimmed();
                sale.time = QDateTime::fromString(list[2].trimmed(), Qt::ISODate);
                sale.total = list[3].toDouble(&totalOk);
                sale.paid = list[4].toDouble(&paidOk);
                sale.change = list[5].toDouble(&changeOk);
                readingSale = true;

                if (!sale.time.isValid() || !totalOk || !paidOk || !changeOk) {
                    writeError(error, QString("销售文件第 %1 行数据错误。").arg(lineNumber));
                    file.close();
                    return false;
                }
            } else if (first == "ITEM") {
                if (!readingSale || list.size() != 6) {
                    writeError(error, QString("销售明细第 %1 行格式错误。").arg(lineNumber));
                    file.close();
                    return false;
                }

                bool priceOk = false;
                bool quantityOk = false;
                bool subtotalOk = false;

                Sale_item item;
                item.productId = list[1].trimmed();
                item.name = list[2].trimmed();
                item.unitPrice = list[3].toDouble(&priceOk);
                item.quantity = list[4].toInt(&quantityOk);
                item.subtotal = list[5].toDouble(&subtotalOk);

                if (!priceOk || !quantityOk || !subtotalOk) {
                    writeError(error, QString("销售明细第 %1 行数字错误。").arg(lineNumber));
                    file.close();
                    return false;
                }

                sale.items.append(item);
            } else if (first == "ENDSALE") {
                if (readingSale) {
                    m_sales.append(sale);
                    sale = Sale();
                    readingSale = false;
                }
            }
        }

        if (readingSale) {
            m_sales.append(sale);
        }

        file.close();
    }

    // 3. 读取挂起订单 suspended_carts.txt
    // 格式：
    // CART|id|time
    // ITEM|productId|quantity
    // ENDCART
    if (QFileInfo::exists(suspendedPath())) {
        QFile file(suspendedPath());

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            writeError(error, "无法读取挂起订单文件：" + suspendedPath());
            return false;
        }

        QTextStream in(&file);
        SuspendedCart cart;
        bool readingCart = false;
        int lineNumber = 0;

        while (!in.atEnd()) {
            QString line = in.readLine();
            lineNumber++;

            if (!isUsefulLine(line)) {
                continue;
            }

            QStringList list = line.split("|");
            QString first = list.value(0);

            if (first == "CART") {
                if (list.size() != 3) {
                    writeError(error, QString("挂起订单第 %1 行格式错误。").arg(lineNumber));
                    file.close();
                    return false;
                }

                cart = SuspendedCart();
                cart.id = list[1].trimmed();
                cart.time = QDateTime::fromString(list[2].trimmed(), Qt::ISODate);
                readingCart = true;

                if (!cart.time.isValid()) {
                    writeError(error, QString("挂起订单第 %1 行时间错误。").arg(lineNumber));
                    file.close();
                    return false;
                }
            } else if (first == "ITEM") {
                if (!readingCart || list.size() != 3) {
                    writeError(error, QString("挂起订单明细第 %1 行格式错误。").arg(lineNumber));
                    file.close();
                    return false;
                }

                bool quantityOk = false;

                Cart_item item;
                item.productId = list[1].trimmed();
                item.quantity = list[2].toInt(&quantityOk);

                if (!quantityOk) {
                    writeError(error, QString("挂起订单明细第 %1 行数量错误。").arg(lineNumber));
                    file.close();
                    return false;
                }

                cart.items.append(item);
            } else if (first == "ENDCART") {
                if (readingCart) {
                    m_suspendedCarts.append(cart);
                    cart = SuspendedCart();
                    readingCart = false;
                }
            }
        }

        if (readingCart) {
            m_suspendedCarts.append(cart);
        }

        file.close();
    }

    rebuildIndex();

    if (needSave) {
        return save(error);
    }

    return true;
}

bool store::save(QString *error) const
{
    QDir dir;
    dir.mkpath(m_dataDir);

    // 1. 保存商品
    QFile productFile(productsPath());
    if (!productFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        writeError(error, "无法保存商品文件：" + productsPath());
        return false;
    }

    QTextStream productOut(&productFile);
    productOut << "# id|name|type|price|number|active\n";

    for (int i = 0; i < m_products.size(); i++) {
        product p = m_products[i];
        productOut << p.id << "|"
                   << p.name << "|"
                   << p.type << "|"
                   << QString::number(p.price, 'f', 2) << "|"
                   << p.number << "|"
                   << (p.active ? 1 : 0)
                   << "\n";
    }

    productFile.close();

    // 2. 保存销售记录
    QFile saleFile(salesPath());
    if (!saleFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        writeError(error, "无法保存销售文件：" + salesPath());
        return false;
    }

    QTextStream saleOut(&saleFile);
    saleOut << "# SALE|id|time|total|paid|change\n";
    saleOut << "# ITEM|productId|name|unitPrice|quantity|subtotal\n";
    saleOut << "# ENDSALE\n";

    for (int i = 0; i < m_sales.size(); i++) {
        Sale sale = m_sales[i];

        saleOut << "SALE|"
                << sale.id << "|"
                << sale.time.toString(Qt::ISODate) << "|"
                << QString::number(sale.total, 'f', 2) << "|"
                << QString::number(sale.paid, 'f', 2) << "|"
                << QString::number(sale.change, 'f', 2)
                << "\n";

        for (int j = 0; j < sale.items.size(); j++) {
            Sale_item item = sale.items[j];

            saleOut << "ITEM|"
                    << item.productId << "|"
                    << item.name << "|"
                    << QString::number(item.unitPrice, 'f', 2) << "|"
                    << item.quantity << "|"
                    << QString::number(item.subtotal, 'f', 2)
                    << "\n";
        }

        saleOut << "ENDSALE\n";
    }

    saleFile.close();

    // 3. 保存挂起订单
    QFile suspendedFile(suspendedPath());
    if (!suspendedFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        writeError(error, "无法保存挂起订单文件：" + suspendedPath());
        return false;
    }

    QTextStream suspendedOut(&suspendedFile);
    suspendedOut << "# CART|id|time\n";
    suspendedOut << "# ITEM|productId|quantity\n";
    suspendedOut << "# ENDCART\n";

    for (int i = 0; i < m_suspendedCarts.size(); i++) {
        SuspendedCart cart = m_suspendedCarts[i];

        suspendedOut << "CART|"
                     << cart.id << "|"
                     << cart.time.toString(Qt::ISODate)
                     << "\n";

        for (int j = 0; j < cart.items.size(); j++) {
            Cart_item item = cart.items[j];

            suspendedOut << "ITEM|"
                         << item.productId << "|"
                         << item.quantity
                         << "\n";
        }

        suspendedOut << "ENDCART\n";
    }

    suspendedFile.close();

    return true;
}

const QList<product> &store::products() const
{
    return m_products;
}

const QList<Sale> &store::sales() const
{
    return m_sales;
}

const QList<SuspendedCart> &store::suspendedCarts() const
{
    return m_suspendedCarts;
}

const product *store::findProduct(const QString &id) const
{
    QString key = id.trimmed();

    if (!m_productIndex.contains(key)) {
        return nullptr;
    }

    int index = m_productIndex.value(key);

    if (index < 0 || index >= m_products.size()) {
        return nullptr;
    }

    return &m_products[index];
}

bool store::addProduct(product product, QString *error)
{
    product.id = product.id.trimmed();
    product.name = product.name.trimmed();
    product.type = product.type.trimmed();

    if (product.type.isEmpty()) {
        product.type = "未分类";
    }

    bool ok = validateProduct(product, true, error);
    if (!ok) {
        return false;
    }

    m_products.append(product);
    rebuildIndex();

    return save(error);
}

bool store::updateProduct(product product, QString *error)
{
    product.id = product.id.trimmed();
    product.name = product.name.trimmed();
    product.type = product.type.trimmed();

    if (product.type.isEmpty()) {
        product.type = "未分类";
    }

    bool ok = validateProduct(product, false, error);
    if (!ok) {
        return false;
    }

    int index = m_productIndex.value(product.id, -1);
    if (index < 0) {
        writeError(error, "商品不存在，无法修改。");
        return false;
    }

    m_products[index] = product;
    rebuildIndex();

    return save(error);
}

bool store::removeProduct(const QString &id, QString *error)
{
    QString key = id.trimmed();

    if (!m_productIndex.contains(key)) {
        writeError(error, "商品不存在，无法删除。");
        return false;
    }

    for (int i = 0; i < m_suspendedCarts.size(); i++) {
        SuspendedCart cart = m_suspendedCarts[i];

        for (int j = 0; j < cart.items.size(); j++) {
            Cart_item item = cart.items[j];

            if (item.productId == key) {
                writeError(error, "该商品存在于挂起订单中，请先处理挂起订单。");
                return false;
            }
        }
    }

    int index = m_productIndex.value(key);
    m_products.removeAt(index);

    rebuildIndex();

    return save(error);
}

bool store::setProductActive(const QString &id, bool active, QString *error)
{
    QString key = id.trimmed();

    if (!m_productIndex.contains(key)) {
        writeError(error, "商品不存在，无法修改上下架状态。");
        return false;
    }

    int index = m_productIndex.value(key);
    m_products[index].active = active;

    return save(error);
}

bool store::validateCart(const QList<Cart_item> &cart, QString *error) const
{
    if (cart.isEmpty()) {
        writeError(error, "购物车为空。");
        return false;
    }

    QHash<QString, int> totalQuantity;

    for (int i = 0; i < cart.size(); i++) {
        Cart_item item = cart[i];

        if (item.quantity <= 0) {
            writeError(error, "购物车存在非法数量。");
            return false;
        }

        int oldValue = totalQuantity.value(item.productId, 0);
        totalQuantity[item.productId] = oldValue + item.quantity;
    }

    QList<QString> productIds = totalQuantity.keys();

    for (int i = 0; i < productIds.size(); i++) {
        QString productId = productIds[i];

        const product *product = findProduct(productId);

        if (product == nullptr) {
            writeError(error, "商品不存在：" + productId);
            return false;
        }

        if (!product->active) {
            writeError(error, "商品已下架：" + product->name);
            return false;
        }

        int needCount = totalQuantity.value(productId);
        if (product->number < needCount) {
            writeError(error, QString("库存不足：%1，当前库存 %2。")
                                  .arg(product->name)
                                  .arg(product->number));
            return false;
        }
    }

    return true;
}

double store::cartTotal(const QList<Cart_item> &cart) const
{
    double total = 0.0;

    for (int i = 0; i < cart.size(); i++) {
        Cart_item item = cart[i];

        const product *product = findProduct(item.productId);
        if (product != nullptr) {
            total = total + product->price * item.quantity;
        }
    }

    return total;
}

int store::cartItemCount(const QList<Cart_item> &cart) const
{
    int count = 0;

    for (int i = 0; i < cart.size(); i++) {
        count = count + cart[i].quantity;
    }

    return count;
}

bool store::commitSale(const QList<Cart_item> &cart, double paid, Sale *createdSale, QString *error)
{
    bool ok = validateCart(cart, error);
    if (!ok) {
        return false;
    }

    double total = cartTotal(cart);

    if (paid + 0.0001 < total) {
        writeError(error, QString("实收金额不足，应收 %1 元。").arg(total, 0, 'f', 2));
        return false;
    }

    Sale sale;
    sale.id = nextSaleId();
    sale.time = QDateTime::currentDateTime();
    sale.total = total;
    sale.paid = paid;
    sale.change = paid - total;

    for (int i = 0; i < cart.size(); i++) {
        Cart_item cartItem = cart[i];

        int index = m_productIndex.value(cartItem.productId, -1);
        if (index < 0) {
            writeError(error, "结算时商品不存在。");
            return false;
        }

        product &product = m_products[index];

        Sale_item saleItem;
        saleItem.productId = product.id;
        saleItem.name = product.name;
        saleItem.unitPrice = product.price;
        saleItem.quantity = cartItem.quantity;
        saleItem.subtotal = product.price * cartItem.quantity;

        sale.items.append(saleItem);

        product.number = product.number - cartItem.quantity;
    }

    m_sales.append(sale);

    if (createdSale != nullptr) {
        *createdSale = sale;
    }

    return save(error);
}

bool store::suspendCart(const QList<Cart_item> &cart, SuspendedCart *createdCart, QString *error)
{
    bool ok = validateCart(cart, error);
    if (!ok) {
        return false;
    }

    SuspendedCart suspended;
    suspended.id = nextSuspendedId();
    suspended.time = QDateTime::currentDateTime();
    suspended.items = cart;

    m_suspendedCarts.append(suspended);

    if (createdCart != nullptr) {
        *createdCart = suspended;
    }

    return save(error);
}

bool store::takeSuspendedCart(const QString &id, QList<Cart_item> *cart, QString *error)
{
    for (int i = 0; i < m_suspendedCarts.size(); i++) {
        if (m_suspendedCarts[i].id == id) {
            if (cart != nullptr) {
                *cart = m_suspendedCarts[i].items;
            }

            m_suspendedCarts.removeAt(i);

            return save(error);
        }
    }

    writeError(error, "未找到挂起订单。");

    return false;
}

QString store::dataDir() const
{
    return m_dataDir;
}

QString store::productsPath() const
{
    QDir dir(m_dataDir);
    return dir.filePath("products.txt");
}

QString store::salesPath() const
{
    QDir dir(m_dataDir);
    return dir.filePath("sales.txt");
}

QString store::suspendedPath() const
{
    QDir dir(m_dataDir);
    return dir.filePath("suspended_carts.txt");
}

void store::rebuildIndex()
{
    m_productIndex.clear();

    for (int i = 0; i < m_products.size(); i++) {
        QString id = m_products[i].id;
        m_productIndex.insert(id, i);
    }
}

void store::seedDefaultProducts()
{
    m_products.clear();

    product p1;
    p1.id = "690001";
    p1.name = "矿泉水";
    p1.type = "饮品";
    p1.price = 2.00;
    p1.number = 120;
    p1.active = true;
    m_products.append(p1);

    product p2;
    p2.id = "690002";
    p2.name = "纯牛奶";
    p2.type = "乳品";
    p2.price = 4.50;
    p2.number = 80;
    p2.active = true;
    m_products.append(p2);

    product p3;
    p3.id = "690003";
    p3.name = "方便面";
    p3.type = "食品";
    p3.price = 5.00;
    p3.number = 90;
    p3.active = true;
    m_products.append(p3);

    product p4;
    p4.id = "690004";
    p4.name = "抽纸";
    p4.type = "日用品";
    p4.price = 12.80;
    p4.number = 50;
    p4.active = true;
    m_products.append(p4);

    product p5;
    p5.id = "690005";
    p5.name = "洗衣液";
    p5.type = "日用品";
    p5.price = 29.90;
    p5.number = 35;
    p5.active = true;
    m_products.append(p5);

    product p6;
    p6.id = "690006";
    p6.name = "苹果";
    p6.type = "生鲜";
    p6.price = 6.80;
    p6.number = 60;
    p6.active = true;
    m_products.append(p6);

    product p7;
    p7.id = "690007";
    p7.name = "下架样品";
    p7.type = "测试";
    p7.price = 1.00;
    p7.number = 5;
    p7.active = false;
    m_products.append(p7);

    rebuildIndex();
}

QString store::nextSaleId() const
{
    QString timeText = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    QString numberText = QString::number(m_sales.size() + 1).rightJustified(4, '0');

    return "S" + timeText + "-" + numberText;
}

QString store::nextSuspendedId() const
{
    QString timeText = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
    QString numberText = QString::number(m_suspendedCarts.size() + 1).rightJustified(3, '0');

    return "P" + timeText + "-" + numberText;
}

bool store::validateProduct(const product &product, bool requireNew, QString *error) const
{
    if (product.id.isEmpty()) {
        writeError(error, "商品编号不能为空。");
        return false;
    }

    if (product.name.isEmpty()) {
        writeError(error, "商品名称不能为空。");
        return false;
    }

    if (product.price < 0.0) {
        writeError(error, "商品价格不能为负数。");
        return false;
    }

    if (product.number < 0) {
        writeError(error, "库存不能为负数。");
        return false;
    }

    bool exists = m_productIndex.contains(product.id);

    if (requireNew && exists) {
        writeError(error, "商品编号已存在。");
        return false;
    }

    if (!requireNew && !exists) {
        writeError(error, "商品编号不存在。");
        return false;
    }

    return true;
}
