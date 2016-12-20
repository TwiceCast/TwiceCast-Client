#-------------------------------------------------
#
# Project created by QtCreator 2016-10-03T16:22:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network
greaterThan(QT_MAJOR_VERSION, 5): QT += websockets

TARGET = TwiceCast
TEMPLATE = app


SOURCES += src/main.cpp \
    src/MainWindow.cpp \
    src/ConnectionWindow.cpp \
    src/ProjectCreationDialog.cpp \
    src/IgnoredAdderDialog.cpp

HEADERS  += include/MainWindow.hpp \
    include/ConnectionWindow.hpp \
    include/ProjectCreationDialog.hpp \
    include/IgnoredAdderDialog.hpp

FORMS    += ui/mainwindow.ui \
    ui/connectionwindow.ui \
    ui/projectcreationdialog.ui \
    ui/ignoredadderdialog.ui

RESOURCES += pictures.qrc

INCLUDEPATH += include

CONFIG += c++11
