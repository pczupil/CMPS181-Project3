#-------------------------------------------------
#
# rmtest_03 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_03
SOURCES = rmtest_03.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
