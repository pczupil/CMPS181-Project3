#-------------------------------------------------
#
# rmtest_extra_3 project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_extra_3
SOURCES = rmtest_extra_3.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
