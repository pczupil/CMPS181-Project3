#-------------------------------------------------
#
# rmtest_02 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_02
SOURCES = rmtest_02.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
