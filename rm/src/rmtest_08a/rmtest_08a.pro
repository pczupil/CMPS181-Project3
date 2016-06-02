#-------------------------------------------------
#
# rmtest_08a project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_08a
SOURCES = rmtest_08a.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
