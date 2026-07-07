#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

class QComboBox;
class QDateEdit;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableWidget;
class QWidget;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QComboBox *roleCombo() const;
    QLineEdit *cashierSearchEdit() const;
    QTableWidget *cashierProductTable() const;
    QTableWidget *cartTable() const;
    QPushButton *checkoutButton() const;
    QTableWidget *productTable() const;
    QLineEdit *idEdit() const;
    QDoubleSpinBox *priceSpin() const;
    QSpinBox *stockSpin() const;
    QDateEdit *reportDayEdit() const;
    QDateEdit *reportMonthEdit() const;
    QLabel *reportSummaryLabel() const;
    void setAdminMode(bool adminMode);
    void setDataPathText(const QString &text);

private:
    QWidget *buildTopBar();
    QWidget *buildCashierPage();
    QWidget *buildProductPage();
    QWidget *buildReportPage();
    QWidget *buildSystemPage();
    void applyStyle();

    Ui::MainWindow *ui;
    QPushButton *m_checkoutButton;
    QDateEdit *m_reportDayEdit;
    QDateEdit *m_reportMonthEdit;
    QLabel *m_reportSummaryLabel;
    QLabel *m_dataPathLabel;
};

#endif // MAINWINDOW_H
