#-------------------------------------------------
#
# Project created by QtCreator 2016-12-14T08:19:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = calliper
TEMPLATE = app


SOURCES += app-calliper/main.cpp\
        app-calliper/widgets/mainwindow/mainwindow.cpp \
    app-calliper/widgets/dock/projectfiledockwidget.cpp \
    app-calliper/widgets/dock/visibleactiondockwidget.cpp \
    app-calliper/model/applicationproject.cpp \
    app-calliper/widgets/dock/projectmetadatadockwidget.cpp \
    app-calliper_global.cpp \
    app-calliper/application/applicationtasks.cpp \
    app-calliper/widgets/projectfiles/projectfiletreewidget.cpp

HEADERS  += app-calliper/widgets/mainwindow/mainwindow.h \
    app-calliper/widgets/dock/projectfiledockwidget.h \
    app-calliper/widgets/dock/visibleactiondockwidget.h \
    app-calliper/model/applicationproject.h \
    app-calliper_global.h \
    app-calliper/widgets/dock/projectmetadatadockwidget.h \
    app-calliper/application/applicationtasks.h \
    app-calliper/widgets/projectfiles/projectfiletreewidget.h

FORMS    += mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../calliperutil/release/ -lcalliperutil
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../calliperutil/debug/ -lcalliperutil
else:unix: LIBS += -L$$OUT_PWD/../calliperutil/ -lcalliperutil

INCLUDEPATH += $$PWD/../calliperutil
DEPENDPATH += $$PWD/../calliperutil

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../renderer/release/ -lrenderer
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../renderer/debug/ -lrenderer
else:unix: LIBS += -L$$OUT_PWD/../renderer/ -lrenderer

INCLUDEPATH += $$PWD/../renderer
DEPENDPATH += $$PWD/../renderer

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../model-loaders/release/ -lmodel-loaders
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../model-loaders/debug/ -lmodel-loaders
else:unix: LIBS += -L$$OUT_PWD/../model-loaders/ -lmodel-loaders

INCLUDEPATH += $$PWD/../model-loaders
DEPENDPATH += $$PWD/../model-loaders

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../dep-vtflib/release/ -ldep-vtflib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../dep-vtflib/debug/ -ldep-vtflib
else:unix: LIBS += -L$$OUT_PWD/../dep-vtflib/ -ldep-vtflib

INCLUDEPATH += $$PWD/../dep-vtflib
DEPENDPATH += $$PWD/../dep-vtflib

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../file-formats/release/ -lfile-formats
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../file-formats/debug/ -lfile-formats
else:unix: LIBS += -L$$OUT_PWD/../file-formats/ -lfile-formats

INCLUDEPATH += $$PWD/../file-formats
DEPENDPATH += $$PWD/../file-formats

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../model/release/ -lmodel
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../model/debug/ -lmodel
else:unix: LIBS += -L$$OUT_PWD/../model/ -lmodel

INCLUDEPATH += $$PWD/../model
DEPENDPATH += $$PWD/../model

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../user-interface/release/ -luser-interface
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../user-interface/debug/ -luser-interface
else:unix: LIBS += -L$$OUT_PWD/../user-interface/ -luser-interface

INCLUDEPATH += $$PWD/../user-interface
DEPENDPATH += $$PWD/../user-interface
