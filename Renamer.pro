#-------------------------------------------------
#
# Project created by QtCreator 2014-03-08T15:45:52
#
#-------------------------------------------------

QT       += core gui multimedia widgets concurrent
CONFIG += c++17

TARGET = Renamer
TEMPLATE = app

win32 {
    RC_FILE = Resource.rc
}
macx {
    ICON = Images/Renamer.icns
}

SOURCES +=\
        MainWindow.cpp \
    Main.cpp \
    Renamer.cpp \
    DlgSettings.cpp \
    Exif.cpp

HEADERS  += MainWindow.h \
    Renamer.h \
    DlgSettings.h \
    Exif.h

FORMS    += MainWindow.ui \
    DlgSettings.ui

RESOURCES += \
    Resources.qrc

DISTFILES +=
