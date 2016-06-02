#-------------------------------------------------
#
# rmtest_09 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_09
SOURCES = rmtest_09.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
