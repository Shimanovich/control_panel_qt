QT       += core gui widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Force g++ (решает atomic)
QMAKE_CXX = g++
QMAKE_LINK = g++
QMAKE_LINK_SHLIB = g++

CONFIG += c++17

DEFINES += QT_NO_ENTRYPOINT

QMAKE_LFLAGS += -Wl,--allow-multiple-definition

# SDL2
SDL2_PATH = d:/work/SDL2-2.0.14/x86_64-w64-mingw32/

INCLUDEPATH += $$SDL2_PATH/include

LIBS += -L$$SDL2_PATH/lib -lSDL2



SOURCES += \
    Settings.cpp \
    cameralogic.cpp \
    gimbalLogic.cpp \
    main.cpp \
    mainwindow.cpp \
    mc108m3camera.cpp \
    networkmanager.cpp \
    sbgcinterface.cpp \
    sbgcprotocol.cpp \
    udpworker.cpp

HEADERS += \
    Settings.h \
    cameralogic.h \
    gimbalLogic.h \
    mainwindow.h \
    mc108m3camera.hpp \
    networkmanager.h \
    sbgc.hpp \
    sbgcinterface.h \
    sbgcprotocol.hpp \
    udpworker.h

FORMS += \
    mainwindow.ui

TRANSLATIONS += \
    controlPanel_ru_BY.ts
CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target