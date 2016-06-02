#-------------------------------------------------
#
# rbftest11b project
#
#-------------------------------------------------

QT       -= gui

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

TARGET = rbftest11b
SOURCES = rbftest11b.cpp
INCLUDEPATH += ../rbf
LIBS += -L../rbf -lrbf
