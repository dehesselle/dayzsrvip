#-------------------------------------------------
#
# Project created by QtCreator 2016-11-17T22:18:10
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = testpgm
TEMPLATE = app


SOURCES += main.cpp\
    ../plugin/DayzServerIp.cpp \
    ../plugin/IniFile.cpp \
    ../plugin/Player.cpp \
    ../plugin/Log.cpp

HEADERS  += ../plugin/DayzServerIp.h \
    ../plugin/IniFile.h \
    ../plugin/Log.h \
    ../plugin/Player.h

FORMS    += ../plugin/DayzServerIp.ui

RESOURCES += \
    ../plugin/dayzsrvip.qrc

DEFINES += DAYZSERVERIP_VERSION=\\\"$$system(C:/Portable/msys64/usr/bin/git --git-dir $$PWD/../.git --work-tree $$PWD describe --always --tags)\\\"

INCLUDEPATH += $$PWD/../plugin
