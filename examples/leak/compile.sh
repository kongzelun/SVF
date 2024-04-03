#!/bin/bash

set -e

. ../../setup.sh

clang -O0 -S -emit-llvm -g leak.c -o noopt.ll
clang -O3 -fno-inline -S -emit-llvm -g leak.c -o opt.ll

saber -leak opt.ll > opt.txt
saber -leak noopt.ll > noopt.txt
