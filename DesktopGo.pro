#-------------------------------------------------
#
# Project created by QtCreator 2018-06-30T14:52:43
#
#-------------------------------------------------

QT       += core gui winextras multimedia multimediawidgets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DesktopGo
TEMPLATE = app


SOURCES += main.cpp \
    hexwidget.cpp \
    dockermanager.cpp \
    videowidget.cpp \
    desktopgo.cpp

HEADERS  += \
    hexwidget.h \
    dockermanager.h \
    videowidget.h \
    desktopgo.h

RESOURCES += \
    res.qrc

DISTFILES += \
    myapp.rc

RC_FILE += myapp.rc

FORMS += \
    setwindow.ui
