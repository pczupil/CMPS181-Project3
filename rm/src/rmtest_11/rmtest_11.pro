#-------------------------------------------------
#
# rmtest_11 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_11
SOURCES = rmtest_11.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
