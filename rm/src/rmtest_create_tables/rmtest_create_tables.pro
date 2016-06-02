#-------------------------------------------------
#
# rmtest_create_tables project
#
#-------------------------------------------------

QT       -= gui

TARGET = rmtest_create_tables
SOURCES = rmtest_create_tables.cpp

CONFIG += debug
CONFIG += c++11

QMAKE_CXXFLAGS += -std=c++11

INCLUDEPATH += ../rm
LIBS += -L../rm -lrm

LIBS += -L../../../rbf -lrbf
