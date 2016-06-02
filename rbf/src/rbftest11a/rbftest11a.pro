#-------------------------------------------------
#
# rbftest11a project
#
#-------------------------------------------------

QT       -= gui

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

TARGET = rbftest11a
SOURCES = rbftest11a.cpp
INCLUDEPATH += ../rbf
LIBS += -L../rbf -lrbf
