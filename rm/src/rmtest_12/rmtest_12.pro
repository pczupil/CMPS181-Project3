#-------------------------------------------------
#
# rmtest_12 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_12
SOURCES = rmtest_12.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
