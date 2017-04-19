#!/bin/bash

export TABLE_PROPAGATOR="hej"

i=0
while [ $i -lt 1 ]
do
    file="fzn/crossword3_${i}_gecode.fzn"
    ${GECODE_PATH}/bin/fzn-gecode -s $file #> "solutions/compact-table_$i.txt"
    #fzn-gecode -s $file > "solutions/gecode_$i.txt"
    i=$(($i + 1))
done
