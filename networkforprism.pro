#-------------------------------------------------
#
# Project created by QtCreator 2015-06-29T19:59:57
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = networkforprism
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    setupwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

QT +=network
QT +=webkitwidgets

RESOURCES += \
    resources.qrc
