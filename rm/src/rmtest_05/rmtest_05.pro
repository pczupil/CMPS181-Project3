#-------------------------------------------------
#
# rmtest_05 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_05
SOURCES = rmtest_05.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
