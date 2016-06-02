#!/bin/bash
unzip $1.zip
cd $1
cd codebase
cd ix
make clean
make
./ixtest1
./ixtest2
