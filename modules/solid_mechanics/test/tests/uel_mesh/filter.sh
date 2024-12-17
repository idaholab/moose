#!/bin/zsh

for n in 0 1 2
do
	grep $1 stdout.processor.$n | cut -d' ' -f2- > out.$n
done
