#-------------------------------------------------
#
# Common ix project
#
#-------------------------------------------------

QT       += core
QT       -= gui

TEMPLATE = lib
CONFIG   = staticlib
CONFIG   += qt
CONFIG   += c++11
CONFIG   += debug

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    ix.cpp

HEADERS += \
    ix.h
