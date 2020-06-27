HEADERS = bimodal_thresh.h
SOURCES = bimodal_thresh.cpp

TARGET  = $$qtLibraryTarget(bimodal_thresh)
DESTDIR = ..
INCLUDEPATH += ../../src

TEMPLATE        = lib
CONFIG         += plugin
QMAKE_CXXFLAGS  = -std=c++11 -fopenmp
QMAKE_LFLAGS   += -s
LIBS           += -lgomp

MOC_DIR =     ../../build
OBJECTS_DIR = ../../build

unix {
    INSTALLS += target
    target.path = /usr/local/share/photoquick/plugins
}

CONFIG -= debug_and_release
