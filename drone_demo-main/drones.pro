QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    canvas.cpp \
    determinant.cpp \
    drone.cpp \
    main.cpp \
    mainwindow.cpp \
    mypolygon.cpp \
    server.cpp \
    triangle.cpp \
    vector2d.cpp \
    voronoi.cpp
HEADERS += \
    canvas.h \
    determinant.h \
    drone.h \
    mainwindow.h \
    mypolygon.h \
    server.h \
    triangle.h \
    vector2d.h \
    voronoi.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    media/compas.png \
    media/stop.png
