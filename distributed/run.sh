#!/bin/sh

rm -r build
mkdir build
mkdir build/services
mkdir build/services/SPLIT
mkdir build/services/GATHER
mkdir build/services/SIMPLIFY
make

./build/simplifier "$(cat /proc/sys/kernel/hostname)" build/services ../../absolute-project/kobe-sat/data/SAT2020/Kakuro-easy-041-ext.xml.hg_4.cnf.xz&
./build/solver build/services&
./build/split "$(cat /proc/sys/kernel/hostname)" build/services&
