#-------------------------------------------------
#
# rmtest_07 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_07
SOURCES = rmtest_07.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
