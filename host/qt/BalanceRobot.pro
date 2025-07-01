
QT       += core gui serialport charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BalanceRobot
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp

HEADERS += \
        mainwindow.h

FORMS += \
        mainwindow.ui

RESOURCES += \
        resources.qrc
