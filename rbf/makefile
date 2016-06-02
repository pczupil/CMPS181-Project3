ifneq (,$(wildcard /usr/local/qt-4.8.6/bin/qmake))
	QMAKE := /usr/local/qt-4.8.6/bin/qmake
else
	QMAKE := qmake-qt4
endif

TESTS = rbftest rbftest11a rbftest11b rbftest11c
GRIND = valgrind --leak-check=full --show-leak-kinds=all

all: librbf.a ${TESTS}

build:
	@(mkdir -p build; \
	cd build; \
	$(QMAKE) ../src/rbf.pro; \
	make)

librbf.a: build
	@(cp build/rbf/librbf.a .)

${TESTS}: build
	@(cp build/$@/$@ .)

clean:
	@(rm -rf build)

spotless : clean
	- rm librbf.a ${TESTS}

again : spotless all

test : all
	./rbftest

testa : all
	- ./rbftest11a

testb : all
	- ./rbftest11b

testc : all
	- ./rbftest11c

grind : all
	${GRIND} ./rbftest
