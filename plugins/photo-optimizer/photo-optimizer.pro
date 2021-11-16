HEADERS = $$files(*.h)
SOURCES = $$files(*.cpp) ../../src/exif.cpp

TARGET  = $$qtLibraryTarget(photo-optimizer)
DESTDIR = ..
INCLUDEPATH += ../../src

TEMPLATE        = lib
CONFIG         += plugin
QMAKE_CXXFLAGS  = -std=c++11
QMAKE_LFLAGS   += -s
LIBS           +=
QT             += widgets

BUILD_DIR =   ../../build
MOC_DIR =     $$BUILD_DIR
RCC_DIR =     $$BUILD_DIR
OBJECTS_DIR = $$BUILD_DIR
UI_DIR  =     $$BUILD_DIR

unix {
    INSTALLS += target
    target.path = /usr/local/share/photoquick/plugins
}

CONFIG -= debug_and_release debug
