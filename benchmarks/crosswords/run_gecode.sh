#!/bin/bash

export TABLE_PROPAGATOR="gecode"

i=0
while [ $i -lt 72 ]
do
    file="crossword3_$i.mzn"
    mzn-gecode -s $file > "solutions/gecode_$i.txt"
    i=$(($i + 1))
done
