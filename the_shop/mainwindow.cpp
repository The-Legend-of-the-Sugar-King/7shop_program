#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDate>
#include <QDateEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_checkoutButton(0),
    m_reportDayEdit(0),
    m_reportMonthEdit(0),
    m_reportSummaryLabel(0),
    m_dataPathLabel(0)
{
    ui->setupUi(this);

    buildTopBar();
    buildCashierPage();
    buildProductPage();
    buildReportPage();
    buildSystemPage();
    applyStyle();

    setWindowTitle(QStringLiteral("超市收银系统"));
    setDataPathText(QStringLiteral("数据目录：./data"));
    setAdminMode(true);

}

MainWindow::~MainWindow()
{
    delete ui;
}

QComboBox *MainWindow::roleCombo() const
{
    return ui->m_roleCombo;
}

QLineEdit *MainWindow::cashierSearchEdit() const
{
    return ui->m_cashierSearchEdit;
}

QTableWidget *MainWindow::cashierProductTable() const
{
    return ui->m_cashierProductTable;
}

QTableWidget *MainWindow::cartTable() const
{
    return ui->m_cartTable;
}

QPushButton *MainWindow::checkoutButton() const
{
    return m_checkoutButton;
}

QTableWidget *MainWindow::productTable() const
{
    return ui->m_productTable;
}

QLineEdit *MainWindow::idEdit() const
{
    return ui->m_idEdit;
}

QDoubleSpinBox *MainWindow::priceSpin() const
{
    return ui->m_priceSpin;
}

QSpinBox *MainWindow::stockSpin() const
{
    return ui->m_stockSpin;
}

QDateEdit *MainWindow::reportDayEdit() const
{
    return m_reportDayEdit;
}

QDateEdit *MainWindow::reportMonthEdit() const
{
    return m_reportMonthEdit;
}

QLabel *MainWindow::reportSummaryLabel() const
{
    return m_reportSummaryLabel;
}

void MainWindow::setAdminMode(bool adminMode)
{
    const int productIndex = ui->m_tabs->indexOf(ui->ProductManagement);
    const int reportIndex = ui->m_tabs->indexOf(ui->SalesStatistics);

    if (productIndex >= 0) {
        ui->m_tabs->setTabEnabled(productIndex, adminMode);
    }

    if (reportIndex >= 0) {
        ui->m_tabs->setTabEnabled(reportIndex, adminMode);
    }

    if (!adminMode) {
        const int currentIndex = ui->m_tabs->currentIndex();
        if (currentIndex == productIndex || currentIndex == reportIndex) {
            const int cashierIndex = ui->m_tabs->indexOf(ui->m_cashier);
            if (cashierIndex >= 0) {
                ui->m_tabs->setCurrentIndex(cashierIndex);
            }
        }
    }
}

void MainWindow::setDataPathText(const QString &text)
{
    buildSystemPage();

    if (m_dataPathLabel) {
        m_dataPathLabel->setText(text);
    }
}

QWidget *MainWindow::buildTopBar()
{
    return ui->TopBar;
}

QWidget *MainWindow::buildCashierPage()
{
    ui->m_quantitySpin->setMinimum(1);
    ui->m_quantitySpin->setMaximum(999999);
    ui->m_paidSpin->setDecimals(2);
    ui->m_paidSpin->setMaximum(99999999.99);

    if (!m_checkoutButton) {
        m_checkoutButton = new QPushButton(QStringLiteral("结算"), ui->m_cashier);
        m_checkoutButton->setObjectName(QStringLiteral("m_checkoutButton"));
        m_checkoutButton->setMinimumHeight(36);
        ui->gridLayout_4->addWidget(m_checkoutButton, 2, 0, 1, 4);
    }

    return ui->m_cashier;
}

QWidget *MainWindow::buildProductPage()
{
    ui->m_priceSpin->setDecimals(2);
    ui->m_priceSpin->setMaximum(99999999.99);
    ui->m_stockSpin->setMaximum(99999999);

    return ui->ProductManagement;
}

QWidget *MainWindow::buildReportPage()
{
    if (!m_reportDayEdit || !m_reportMonthEdit || !m_reportSummaryLabel) {
        QWidget *toolbar = new QWidget(ui->SalesStatistics);
        QHBoxLayout *layout = new QHBoxLayout(toolbar);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(8);

        QLabel *dayLabel = new QLabel(QStringLiteral("日统计："), toolbar);
        m_reportDayEdit = new QDateEdit(QDate::currentDate(), toolbar);
        m_reportDayEdit->setCalendarPopup(true);
        m_reportDayEdit->setDisplayFormat(QStringLiteral("yyyy-MM-dd"));

        QLabel *monthLabel = new QLabel(QStringLiteral("月统计："), toolbar);
        m_reportMonthEdit = new QDateEdit(QDate::currentDate(), toolbar);
        m_reportMonthEdit->setCalendarPopup(true);
        m_reportMonthEdit->setDisplayFormat(QStringLiteral("yyyy-MM"));

        m_reportSummaryLabel = new QLabel(QStringLiteral("统计摘要：暂无数据"), toolbar);
        m_reportSummaryLabel->setObjectName(QStringLiteral("m_reportSummaryLabel"));

        layout->addWidget(dayLabel);
        layout->addWidget(m_reportDayEdit);
        layout->addSpacing(12);
        layout->addWidget(monthLabel);
        layout->addWidget(m_reportMonthEdit);
        layout->addStretch();
        layout->addWidget(m_reportSummaryLabel);

        ui->gridLayout_12->addWidget(toolbar, 2, 0, 1, 2);
    }

    return ui->SalesStatistics;
}

QWidget *MainWindow::buildSystemPage()
{
    QTextBrowser *browser = ui->textBrowser;
    browser->setOpenExternalLinks(true);
    browser->setHtml(QStringLiteral(
        "<h2>超市收银系统</h2>"
        "<p>当前前端负责界面布局、控件暴露和页面切换。</p>"
        "<p>业务计算、库存判断、订单保存应由后端完成。</p>"));

    if (!m_dataPathLabel) {
        m_dataPathLabel = new QLabel(QStringLiteral("数据目录：未设置"), ui->SystemDescription);
        m_dataPathLabel->setObjectName(QStringLiteral("m_dataPathLabel"));
        m_dataPathLabel->setWordWrap(true);
        ui->gridLayout_11->addWidget(m_dataPathLabel, 1, 0);
    }

    return ui->SystemDescription;
}

void MainWindow::applyStyle()
{
    setStyleSheet(QStringLiteral(
        "QMainWindow { background: #f3f4f6; }"
        "QTabWidget::pane { border: 1px solid #d1d5db; background: #ffffff; border-radius: 8px; }"
        "QTabBar::tab { background: #e5e7eb; color: #334155; padding: 10px 18px; margin-right: 4px; border-top-left-radius: 8px; border-top-right-radius: 8px; }"
        "QTabBar::tab:selected { background: #ffffff; color: #0f172a; }"
        "QLabel { color: #0f172a; }"
        "QLineEdit, QSpinBox, QDoubleSpinBox, QDateEdit, QComboBox, QTextBrowser, QTableWidget, QTableView { background: #ffffff; border: 1px solid #cbd5e1; border-radius: 6px; padding: 4px; selection-background-color: #0ea5e9; }"
        "QPushButton { background: #0f766e; color: #ffffff; border: none; border-radius: 6px; padding: 8px 14px; }"
        "QPushButton:hover { background: #115e59; }"
        "QHeaderView::section { background: #e2e8f0; color: #334155; padding: 6px; border: none; border-bottom: 1px solid #cbd5e1; }"
        "QLabel#m_reportSummaryLabel, QLabel#m_dataPathLabel { color: #475569; font-weight: 600; }"));
}
