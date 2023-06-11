QT += core network gui widgets

TARGET = client
TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS


CONFIG += debug_and_release

SOURCES += \
    ../sources/main.cpp \
    ../sources/chatwindow.cpp \
    ../sources/chatclient.cpp

FORMS += \
    ../forms/chatwindow.ui

HEADERS += \
    ../include/chatwindow.h \
    ../include/chatclient.h

INCLUDEPATH += ../include
