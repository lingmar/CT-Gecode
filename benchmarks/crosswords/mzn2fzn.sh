#!/bin/bash

export TABLE_PROPAGATOR=$1

i=0
while [ $i -lt 72 ]
do
    file="mzn/crossword3_$i.mzn"
    mzn2fzn $file > "fzn/crossword3_${i}_$TABLE_PROPAGATOR.fzn"
    i=$(($i + 1))
done
