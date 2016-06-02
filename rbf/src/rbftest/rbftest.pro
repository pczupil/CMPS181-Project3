#-------------------------------------------------
#
# rbftest project
#
#-------------------------------------------------

QT       -= gui

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

TARGET = rbftest
SOURCES = rbftest.cpp
INCLUDEPATH += ../rbf
LIBS += -L../rbf -lrbf
