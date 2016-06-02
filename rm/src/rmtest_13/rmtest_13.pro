#-------------------------------------------------
#
# rmtest_13 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_13
SOURCES = rmtest_13.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
