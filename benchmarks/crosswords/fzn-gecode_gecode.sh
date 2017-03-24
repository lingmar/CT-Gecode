#!/bin/bash

export TABLE_PROPAGATOR="gecode"

i=0
while [ $i -lt 72 ]
do
    file="fzn/crossword3_${i}_gecode.fzn"
    ${GECODE_PATH}/bin/fzn-gecode -s $file > "solutions/gecode_$i.txt"
    #fzn-gecode -s $file > "solutions/gecode_$i.txt"
    i=$(($i + 1))
done
