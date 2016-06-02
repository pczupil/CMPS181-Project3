ifneq (,$(wildcard /usr/local/qt-4.8.6/bin/qmake))
	QMAKE := /usr/local/qt-4.8.6/bin/qmake
else
	QMAKE := qmake-qt4
endif

TESTS = rmtest_00 rmtest_01 rmtest_02 rmtest_03 rmtest_04 rmtest_05 rmtest_06 rmtest_07 rmtest_08a rmtest_08b rmtest_09 rmtest_10 rmtest_11 rmtest_12 rmtest_13 rmtest_14 rmtest_15 rmtest_16 rmtest_create_tables rmtest_extra_1 rmtest_extra_2 rmtest_extra_3
GRIND = valgrind --leak-check=full --show-leak-kinds=all

all: ../rbf/librbf.a librm.a ${TESTS}

../rbf/librbf.a:
	@(cd ../rbf; \
	make)

build:
	@(mkdir -p build; \
	cd build; \
	$(QMAKE) ../src/rm.pro; \
	make)

librm.a: build
	@(cp build/rm/librm.a .)

${TESTS}: build
	@(cp build/$@/$@ .)

clean:
	@(rm -rf build)

spotless : clean
	- rm -f librm.a ${TESTS} *.bitch

again : spotless all

test : all
	- ./rmtest_create_tables
	- (for test in ${TESTS}; do ./$$test; done;)

grind : all
	- ./rmtest_create_tables
	- (for test in ${TESTS}; do ${GRIND} ./$$test; done;)
