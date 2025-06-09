#-------------------------------------------------
#
# Project created by QtCreator 2013-04-12T11:06:05
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = eZ-FET_Production_Setup
TEMPLATE = app

INCLUDEPATH += ".\qextserialport-1.2rc\src"

SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h \
    MSP430_FET.h \
    MSP430_EEM.h \
    MSP430_Debug.h \
    MSP430.h \
    HIL.h

FORMS += mainwindow.ui

#QMAKE_CXXFLAGS += -m32
#QMAKE_LFLAGS += -m32

#LIBPATH += "C:\Users\a0406477\Documents\Projects\UIF+\Production_Setup\eZ-FET_Production_Setup"
#LIBS += "C:\Users\a0406477\Documents\Projects\UIF+\Production_Setup\eZ-FET_Production_Setup\MSP430.lib"
#LIBS += "MSP430.lib"
#LIBS += -l"C:\Users\a0406477\Documents\Projects\UIF+\Production_Setup\eZ-FET_Production_Setup\MSP430"

include(qextserialport-1.2rc/src/qextserialport.pri)
