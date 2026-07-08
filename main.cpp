#include "mainwindow.h"
#include "maincontroller.h"
#include <QApplication>

// ============================================================
//  main.cpp —— 组装前端 + 控制器
//
//  后端接入时，在这里创建后端对象传给 MainController：
//    RealProductBackend *pb = new RealProductBackend;
//    MainController controller(&w, pb, ob, sb, ub);
//  然后把 MainController 里的 m_products 等删掉，改为调后端 API
// ============================================================
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;

    MainController controller(&w);
    controller.init();

    w.show();

    return a.exec();
}
