#!/bin/bash

for file in *.plt;
do
foo=`basename -s .plt $file`.eps;
echo $foo
gnuplot $file > $foo;
done
