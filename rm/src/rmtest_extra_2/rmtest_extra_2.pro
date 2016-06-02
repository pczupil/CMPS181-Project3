#-------------------------------------------------
#
# rmtest_extra_2 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_extra_2
SOURCES = rmtest_extra_2.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
