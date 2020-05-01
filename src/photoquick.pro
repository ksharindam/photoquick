TEMPLATE = app
TARGET = photoquick
DEPENDPATH += .
INCLUDEPATH += .
QMAKE_CXXFLAGS = -fopenmp -std=c++11
QMAKE_LFLAGS += -s
LIBS += -lgomp
win32:DEFINES += WIN32

CONFIG -= debug

# build dir
MOC_DIR = build
RCC_DIR = build
UI_DIR = build
OBJECTS_DIR = build
mytarget.commands += $${QMAKE_MKDIR} build

# Input
HEADERS += canvas.h main.h transform.h photogrid.h dialogs.h inpaint.h iscissor.h
SOURCES += common.cpp exif.cpp canvas.cpp main.cpp transform.cpp dialogs.cpp \
        photogrid.cpp filters.cpp pdfwriter.cpp inpaint.cpp iscissor.cpp
RESOURCES += resources.qrc
FORMS += mainwindow.ui resize_dialog.ui photogrid_dialog.ui gridsetup_dialog.ui \
         collage_dialog.ui collagesetup_dialog.ui inpaint_dialog.ui iscissor_dialog.ui

# install
!win32 {
    INSTALLS += target
    target.path = /usr/local/bin
}

