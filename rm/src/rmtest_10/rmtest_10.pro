#-------------------------------------------------
#
# rmtest_10 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_10
SOURCES = rmtest_10.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
