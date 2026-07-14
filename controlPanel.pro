QT       += core gui widgets network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Settings.cpp \
    cameralogic.cpp \
    main.cpp \
    mainwindow.cpp \
    mc108m3camera.cpp \
    udpworker.cpp

HEADERS += \
    Settings.h \
    cameralogic.h \
    mainwindow.h \
    mc108m3camera.hpp \
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
