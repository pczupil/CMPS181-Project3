#-------------------------------------------------
#
# rmtest_04 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_04
SOURCES = rmtest_04.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
