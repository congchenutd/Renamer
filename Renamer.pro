#-------------------------------------------------
#
# Project created by QtCreator 2014-03-08T15:45:52
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

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
    DlgSettings.cpp

HEADERS  += MainWindow.h \
    Renamer.h \
    DlgSettings.h

FORMS    += MainWindow.ui \
    DlgSettings.ui

RESOURCES += \
    Resources.qrc
