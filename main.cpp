#include "mainwindow.h"
#include "maincontroller.h"
#include "logindialog.h"
#include <QApplication>
#include <QDialog>

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

    LoginDialog loginDialog;

    QObject::connect(&loginDialog, &LoginDialog::SIG_LoginCommit,
                     &loginDialog,
                     [&loginDialog](QString username, QString password)
                     {
                         Q_UNUSED(username)
                         Q_UNUSED(password)
                         loginDialog.accept();
                     });

    QObject::connect(&loginDialog, &LoginDialog::SIG_RegisterCommit,
                     &loginDialog,
                     [&loginDialog](QString username, QString tel, QString password)
                     {
                         Q_UNUSED(username)
                         Q_UNUSED(tel)
                         Q_UNUSED(password)
                         loginDialog.showErrorTips("注册信息已提交，请返回登录", "success");
                     });

    QObject::connect(&loginDialog, &LoginDialog::SIG_CloseLoginDialog,
                     &loginDialog,
                     [&loginDialog]()
                     {
                         loginDialog.reject();
                     });

    if (loginDialog.exec() != QDialog::Accepted)
    {
        return 0;
    }

    MainWindow w;

    MainController controller(&w);
    controller.init();

    w.show();

    return a.exec();
}
