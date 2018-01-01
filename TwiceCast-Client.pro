#-------------------------------------------------
#
# Project created by QtCreator 2016-10-03T16:22:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4){
    QT += widgets network
    greaterThan(QT_MINOR_VERSION, 2): QT += websockets
}

TARGET = TwiceCast
TEMPLATE = app

SOURCES += src/main.cpp \
    src/MainWindow.cpp \
    src/ConnectionWindow.cpp \
    src/ProjectCreationDialog.cpp \
    src/IgnoredAdderDialog.cpp \
    src/NetworkManager.cpp \
    src/Project.cpp \
    src/ProjectOpenDialog.cpp \
    src/User.cpp \
    src/CustomStyle.cpp \
    src/Stream.cpp \
    src/StreamsDialog.cpp \
    src/CustomPRItem.cpp \
    src/MergeDialog.cpp

HEADERS  += include/MainWindow.hpp \
    include/ConnectionWindow.hpp \
    include/ProjectCreationDialog.hpp \
    include/IgnoredAdderDialog.hpp \
    include/NetworkManager.hpp \
    include/Project.hpp \
    include/ProjectOpenDialog.hpp \
    include/User.hpp \
    include/CustomStyle.hpp \
    include/Stream.hpp \
    include/StreamsDialog.hpp \
    include/CustomPRItem.hpp \
    include/MergeDialog.hpp

FORMS    += ui/mainwindow.ui \
    ui/connectionwindow.ui \
    ui/projectcreationdialog.ui \
    ui/ignoredadderdialog.ui \
    ui/projectopendialog.ui \
    ui/streamsdialog.ui \
    ui/mergedialog.ui

RESOURCES += pictures.qrc

INCLUDEPATH += include

Release:DEFINES += QT_NO_DEBUG_OUTPUT

CONFIG += c++11

Debug:DESTDIR = debug
Release:DESTDIR = release

!contains(QMAKE_TARGET.arch, x86_64) {
    win32 : Release:DESTDIR = $${PWD}/build/win32
    win32-g++ : Release:DESTDIR = $${PWD}/build/linux32
    unix : !macx : Release:DESTDIR = $${PWD}/build/linux32
    macx : Release:DESTDIR = $${PWD}/build/macos32
} else {
    win32 : Release:DESTDIR = $${PWD}/build/win64
    win32-g++ : Release:DESTDIR = $${PWD}/build/linux64
    unix : !macx : Release:DESTDIR = $${PWD}/build/linux64
    macx : Release:DESTDIR = $${PWD}/build/macos64
}

win32 : Release:QMAKE_POST_LINK = windeployqt $$DESTDIR
win32-g++ : Release:QMAKE_POST_LINK = windeployqt $$DESTDIR

target.path = $$DESTDIR

INSTALLS += target
