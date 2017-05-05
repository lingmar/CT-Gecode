#!/bin/bash
FZN=$1
EXT=$2

echo $FZN

OUT=$(echo $FZN | rev | cut -f 2- -d '.' | rev)-$EXT.res

export TABLE_PROPAGATOR=compact-table
${GECODE_PATH}/bin/fzn-gecode -time 300000 -s $FZN > $OUT



