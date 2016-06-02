#-------------------------------------------------
#
# Top level project
#
#-------------------------------------------------

QT       += core
QT       -= gui

TARGET   = rbf

CONFIG   += debug

QMAKE_CXXFLAGS += -std=c++11

TEMPLATE = subdirs

SUBDIRS = rbf rbftest rbftest11a rbftest11b rbftest11c
    rbftest.depends = rbf
    rbftest11a.depends = rbf
    rbftest11b.depends = rbf
    rbftest11c.depends = rbf
