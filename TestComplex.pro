#-------------------------------------------------
#
# Project created by QtCreator 2018-07-02T10:29:43
#
#-------------------------------------------------

QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TestComplex
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        test.cpp \
        settingsdialog.cpp \
        commandLine.cpp \
        mytreeitem.cpp \
        mytreemodel.cpp \
        mytreeview.cpp \
        testingdialog.cpp \
    polyfunctions.cpp


HEADERS += \
        mainwindow.h \
        test.h \
        settingsdialog.h \
        commandLine.h \
        mytreeitem.h \
        mytreemodel.h \
        mytreeview.h \
        testingdialog.h \
    polyfunctions.h


FORMS += \
        mainwindow.ui \
        settingsdialog.ui \
        testingdialog.ui

RESOURCES += \
        editabletreemodel.qrc

RC_ICONS = myappmain.ico
