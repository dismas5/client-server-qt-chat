QT += core network gui widgets

TARGET = server
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    ../sources/main.cpp \
    ../sources/chatserver.cpp \
    ../sources/serverworker.cpp \
    ../sources/serverwindow.cpp

HEADERS += \
    ../include/chatserver.h \
    ../include/chatserver.h \
    ../include/serverworker.h \
    ../include/serverwindow.h

CONFIG += debug_and_release

INCLUDEPATH += ../include

FORMS += \
    ../forms/serverwindow.ui
