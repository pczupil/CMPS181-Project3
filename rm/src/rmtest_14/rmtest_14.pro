#-------------------------------------------------
#
# rmtest_14 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_14
SOURCES = rmtest_14.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
