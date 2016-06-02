#-------------------------------------------------
#
# Common rbf project
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
    pfm.cpp \
    rbfm.cpp

HEADERS += \
    pfm.h \
    rbfm.h
