#-------------------------------------------------
#
# rmtest_06 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_06
SOURCES = rmtest_06.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
