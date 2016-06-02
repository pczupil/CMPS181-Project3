#-------------------------------------------------
#
# rmtest_01 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_01
SOURCES = rmtest_01.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
