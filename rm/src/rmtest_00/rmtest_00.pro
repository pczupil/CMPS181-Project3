#-------------------------------------------------
#
# rmtest_00 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_00
SOURCES = rmtest_00.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11


INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
