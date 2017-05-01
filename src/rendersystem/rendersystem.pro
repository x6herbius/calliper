#-------------------------------------------------
#
# Project created by QtCreator 2017-04-26T12:44:58
#
#-------------------------------------------------

QT       += opengl

QT       -= gui

TARGET = rendersystem
TEMPLATE = lib

DEFINES += RENDERSYSTEM_LIBRARY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += rendersystem/global/rendersystem.cpp \
    rendersystem_global.cpp \
    rendersystem/interface-classes/currentcontextguard/contextreferencecounter.cpp \
    rendersystem/private/test/testinterface.cpp \
    rendersystem/endpoints/testinterfaceendpoint.cpp \
    rendersystem/private/stores/opengltexturestore/opengltexturestore.cpp \
    rendersystem/private/opengl/openglerrors.cpp \
    rendersystem/private/shaders/common/vertexformat.cpp \
    rendersystem/private/opengl/opengluniformbuffer.cpp \
    rendersystem/private/opengl/openglvertexarrayobject.cpp \
    rendersystem/private/shaders/base/openglshaderprogram.cpp \
    rendersystem/private/stores/openglshaderstore/openglshaderstore.cpp \
    rendersystem/private/stores/opengltexturestore/namedopengltexture.cpp \
    rendersystem/private/shaders/common/globalshaderuniforms.cpp \
    rendersystem/private/shaders/derived/simplelitshader.cpp \
    rendersystem/private/shaders/derived/unlitpervertexcolorshader.cpp \
    rendersystem/interface-classes/rendermaterial/rendermaterial.cpp \
    rendersystem/private/shaders/common/privateshaderdefs.cpp \
    rendersystem/private/stores/materialstore/materialstore.cpp \
    rendersystem/interface-classes/geometry/geometrysection.cpp \
    rendersystem/interface-classes/geometry/geometrybuilder.cpp \
    rendersystem/private/rendermodel/raw-data/objectsectiongeometrydata.cpp \
    rendersystem/private/rendermodel/batching/openglbuffercollection.cpp \
    rendersystem/private/rendermodel/batching/bufferdatacontainer.cpp

HEADERS += rendersystem/global/rendersystem.h\
        rendersystem_global.h \
    rendersystem/private/store-classes/objectstore.h \
    rendersystem/interface-classes/currentcontextguard/currentcontextguard.h \
    rendersystem/interface-classes/currentcontextguard/contextreferencecounter.h \
    rendersystem/interfaces/itestinterface.h \
    rendersystem/private/test/testinterface.h \
    rendersystem/endpoints/testinterfaceendpoint.h \
    rendersystem/private/store-classes/objectstoreitempointer.h \
    rendersystem/private/stores/opengltexturestore/opengltexturestore.h \
    rendersystem/interfaces/itexturestore.h \
    rendersystem/private/opengl/openglerrors.h \
    rendersystem/private/shaders/common/vertexformat.h \
    rendersystem/private/opengl/openglhelpers.h \
    rendersystem/private/opengl/opengluniformbuffer.h \
    rendersystem/private/opengl/openglvertexarrayobject.h \
    rendersystem/private/shaders/common/ishaderspec.h \
    rendersystem/private/shaders/base/openglshaderprogram.h \
    rendersystem/private/store-classes/objectstoreitem.h \
    rendersystem/private/store-classes/itembasedobjectstore.h \
    rendersystem/private/store-classes/itempointerbasedobjectstore.h \
    rendersystem/private/stores/openglshaderstore/openglshaderstore.h \
    rendersystem/private/stores/opengltexturestore/namedopengltexture.h \
    rendersystem/private/shaders/common/globalshaderuniforms.h \
    rendersystem/private/shaders/derived/simplelitshader.h \
    rendersystem/private/store-classes/staticobjectstore.h \
    rendersystem/private/shaders/derived/unlitpervertexcolorshader.h \
    rendersystem/interface-classes/rendermaterial/rendermaterial.h \
    rendersystem/private/shaders/common/privateshaderdefs.h \
    rendersystem/interface-classes/shader/publicshaderdefs.h \
    rendersystem/interface-classes/texture/publictexturedefs.h \
    rendersystem/private/stores/materialstore/materialstore.h \
    rendersystem/interface-classes/store-defs/publicstoredefs.h \
    rendersystem/interface-classes/geometry/geometrysection.h \
    rendersystem/interface-classes/geometry/geometrybuilder.h \
    rendersystem/private/rendermodel/raw-data/objectsectiongeometrydata.h \
    rendersystem/private/rendermodel/batching/openglbuffercollection.h \
    rendersystem/private/rendermodel/batching/bufferdatacontainer.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

RESOURCES += \
    rendersystem/private/resource/resource.qrc
