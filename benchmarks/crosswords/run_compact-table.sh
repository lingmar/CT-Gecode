#!/bin/bash

export TABLE_PROPAGATOR="compact-table"

i=0
while [ $i -lt 72 ]
do
    file="mzn/crossword3_$i.mzn"
    ${MZN_PATH}/mzn-fzn -f ${GECODE_PATH}/bin/fzn-gecode -I ${GECODE_PATH}/share/gecode/mznlib -Ggecode -s $file > "solutions/compact-table_$i.txt"
    #fzn-gecode -s $file > "solutions/gecode_$i.txt"
    i=$(($i + 1))
done
