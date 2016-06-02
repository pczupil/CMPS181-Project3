#-------------------------------------------------
#
# rbftest11c project
#
#-------------------------------------------------

QT       -= gui

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

TARGET = rbftest11c
SOURCES = rbftest11c.cpp
INCLUDEPATH += ../rbf
LIBS += -L../rbf -lrbf
