#-------------------------------------------------
#
# Top level project
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET   = rm
CONFIG   += console
CONFIG   -= app_bundle
CONFIG   += debug

TEMPLATE = subdirs

SUBDIRS = rm rmtest_00 rmtest_01 rmtest_02 rmtest_03 rmtest_04 rmtest_05 rmtest_06 rmtest_07 rmtest_08a rmtest_08b rmtest_09 rmtest_10 rmtest_11 rmtest_12 rmtest_13 rmtest_14 rmtest_15 rmtest_16 rmtest_create_tables rmtest_extra_1 rmtest_extra_2 rmtest_extra_3

    rmtest_00.depends = rm
	rmtest_01.depends = rm
	rmtest_02.depends = rm
	rmtest_03.depends = rm
	rmtest_04.depends = rm
	rmtest_05.depends = rm
	rmtest_06.depends = rm
	rmtest_07.depends = rm
	rmtest_08a.depends = rm
	rmtest_08b.depends = rm
	rmtest_09.depends = rm
	rmtest_10.depends = rm
	rmtest_11.depends = rm
	rmtest_12.depends = rm
	rmtest_13.depends = rm
	rmtest_14.depends = rm
	rmtest_15.depends = rm
	rmtest_16.depends = rm
	rmtest_create_tables.depends = rm
	rmtest_extra_1.depends = rm
	rmtest_extra_2.depends = rm
	rmtest_extra_3.depends = rm
