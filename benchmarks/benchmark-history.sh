#!/bin/bash

commits='38bb6f5
871c986
11ed3ca
2b700ae
99ee50c
04b460b
d38230a
f2e9a01
5bbb7d6
0e82310
b3c87a1
3527696
54916c4
6e3564f
3010a37
7211eb3
3ad0b50
0bd26a0
26dc286
87b440f
667706e
736baef
4f4ce86
ed7e7fc
bc4e4bd
e5801ef
f2d41b2
345c7a5
a122953
447b455
86ced0d
44d2f34'

for commit in $commits; do
    git checkout $commit \
        && (cd ../test && make -j60) \
        && ./benchmarkk.py --run --benchlist bench.listt
done

