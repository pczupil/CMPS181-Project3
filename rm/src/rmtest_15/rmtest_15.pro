#-------------------------------------------------
#
# rmtest_15 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_15
SOURCES = rmtest_15.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
