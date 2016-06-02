#-------------------------------------------------
#
# Top level project
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET   = ix
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += debug

TEMPLATE = subdirs

SUBDIRS = ix ixtest1 ixtest2

    ixtest1.depends = ix
	ixtest2.depends = ix
