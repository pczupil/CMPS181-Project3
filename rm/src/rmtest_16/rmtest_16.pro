#-------------------------------------------------
#
# rmtest_16 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_16
SOURCES = rmtest_16.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
