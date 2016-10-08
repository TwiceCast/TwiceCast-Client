#-------------------------------------------------
#
# Project created by QtCreator 2016-10-03T16:22:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TwiceCast-Client
TEMPLATE = app


SOURCES += src/main.cpp \
    src/MainWindow.cpp \
    src/ConnectionWindow.cpp

HEADERS  += include/MainWindow.hpp \
    include/ConnectionWindow.hpp

FORMS    += ui/mainwindow.ui \
    ui/connectionwindow.ui

RESOURCES += \
    pictures.qrc
