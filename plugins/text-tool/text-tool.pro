HEADERS = text_tool.h
SOURCES = text_tool.cpp
FORMS += $$files(*.ui)
RESOURCES += $$files(*.qrc)

TARGET  = $$qtLibraryTarget(text-tool)
DESTDIR = ..
INCLUDEPATH += ../../src

TEMPLATE        = lib
CONFIG         += plugin
QMAKE_CXXFLAGS  = -std=c++11 -fopenmp
QMAKE_LFLAGS   += -s
LIBS           += -lgomp
QT             += widgets

BUILD_DIR = ../../build
MOC_DIR =     $$BUILD_DIR
RCC_DIR =     $$BUILD_DIR
OBJECTS_DIR = $$BUILD_DIR
UI_DIR  =     ../build

unix {
    INSTALLS += target
    target.path = /usr/local/share/photoquick/plugins
}

CONFIG -= debug_and_release debug
