#-------------------------------------------------
#
# Common rm project
#
#-------------------------------------------------

TEMPLATE = lib
CONFIG   = staticlib
CONFIG   += qt
CONFIG   += c++11
CONFIG   += debug

QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    rm.cpp

HEADERS += \
    rm.h
