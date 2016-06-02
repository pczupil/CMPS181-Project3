ifneq (,$(wildcard /usr/local/qt-4.8.6/bin/qmake))
	QMAKE := /usr/local/qt-4.8.6/bin/qmake
else
	QMAKE := qmake-qt4
endif

TESTS = ixtest1 ixtest2
GRIND = valgrind --leak-check=full --show-leak-kinds=all

all: ../rbf/librbf.a libix.a ${TESTS}

../rbf/librbf.a:
	@(cd ../rbf; \
	make)

build:
	@(mkdir -p build; \
	cd build; \
	$(QMAKE) ../src/ix.pro; \
	make)

libix.a: build
	@(cp build/ix/libix.a .)

${TESTS}: build
	@(cp build/$@/$@ .)

clean:
	@(rm -rf build)

spotless : clean
	- rm -f libix.a ${TESTS} *.bitch Age_idx Height_idx

again : spotless all

test : all
	- (for test in ${TESTS}; do ./$$test; done;)

grind : all
	- (for test in ${TESTS}; do ${GRIND} ./$$test; done;)
