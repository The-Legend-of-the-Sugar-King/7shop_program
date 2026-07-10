QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Qt 5.6.1 MinGW 32bit 兼容
QMAKE_CXXFLAGS += -std=c++17

TARGET = market
TEMPLATE = app


SOURCES += main.cpp \
        data_persist.cpp \
        mainwindow.cpp \
        maincontroller.cpp \
        logindialog.cpp

HEADERS  += mainwindow.h \
        data_persist.h \
        data_struct.h \
        maincontroller.h \
        logindialog.h \
        passwordedit.h

FORMS    += mainwindow.ui

FORMS    += logindialog.ui

RESOURCES += res.qrc
