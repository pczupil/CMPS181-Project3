#-------------------------------------------------
#
# rmtest_08b project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_08b
SOURCES = rmtest_08b.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
