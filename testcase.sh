#!/bin/bash
timelimit -T 2 -t 1 ../prog4 < ../../tests/$1 2> $1.err > $1.out
cmp ../../tests/$1.ok $1.out
if (($?)); then
	echo "Wrong output!";
	exit 1;
fi;
exit 0;
