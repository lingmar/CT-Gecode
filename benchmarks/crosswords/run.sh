#!/bin/bash

export TABLE_PROPAGATOR="compact_table"

i=0
while [ $i -lt 72 ]
do
    file="crossword3_$i.mzn"
    mzn-gecode -s $file > "solutions/compact_table$i.txt"
    i=$(($i + 1))
done




