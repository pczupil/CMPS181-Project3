#-------------------------------------------------
#
# ixtest2 project
#
#-------------------------------------------------

QT       -= gui

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

TARGET = ixtest2
SOURCES = ixtest2.cpp
INCLUDEPATH += ../ix
LIBS += -L../ix -lix

LIBS += -L../../../rbf -lrbf
