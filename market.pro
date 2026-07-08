QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Qt 5.6.1 MinGW 32bit 兼容
QMAKE_CXXFLAGS += -std=c++11

TARGET = market
TEMPLATE = app


SOURCES += main.cpp \
        mainwindow.cpp \
        maincontroller.cpp

HEADERS  += mainwindow.h \
        maincontroller.h

FORMS    += mainwindow.ui
