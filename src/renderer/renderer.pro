#-------------------------------------------------
#
# Project created by QtCreator 2016-08-17T21:21:29
#
#-------------------------------------------------

QT       += opengl

TARGET = renderer
TEMPLATE = lib

DEFINES += RENDERER_LIBRARY

SOURCES += \
    demoglwindow.cpp

HEADERS +=\
        renderer_global.h \
    demoglwindow.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}