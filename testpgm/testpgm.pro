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
    ../plugin/DebugDialog.cpp \
    ../plugin/IniFile.cpp \
    ../plugin/Player.cpp \
    ../plugin/Log.cpp

HEADERS  += ../plugin/DayzServerIp.h \
    ../plugin/DebugDialog.h \
    ../plugin/IniFile.h \
    ../plugin/Log.h \
    ../plugin/Player.h \
    ../plugin/Version.h

FORMS    += ../plugin/DayzServerIp.ui \
    ../plugin/DebugDialog.ui

RESOURCES += \
    ../plugin/dayzsrvip.qrc

INCLUDEPATH += $$PWD/../plugin
