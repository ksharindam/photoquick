TEMPLATE = app
TARGET = photoquick
DEPENDPATH += .
INCLUDEPATH += .
QMAKE_CXXFLAGS = -fopenmp -std=c++11
QMAKE_LFLAGS += -s
LIBS += -lgomp

CONFIG -= debug_and_release

# build dir
UI_DIR  =     ../build
MOC_DIR =     ../build
RCC_DIR =     ../build
OBJECTS_DIR = ../build
mytarget.commands += $${QMAKE_MKDIR} ../build

# Input
HEADERS += $$files(*.h)

SOURCES += $$files(*.cpp)

FORMS += $$files(*.ui)

RESOURCES += resources.qrc

# install
!win32 {
    INSTALLS += target
    target.path = /usr/local/bin
}

