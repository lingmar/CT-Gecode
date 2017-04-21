#!/bin/sh

# Script for converting mzn to fzn

D=`dirname $1`
D=$(echo $D | cut -f 2 -d '/')
T=$1
#du -h $1
unlzma $T
#du -h ${T%.*}
#echo $1
MZN=${T%.*}
#echo $D $XML > $(echo $XML | rev | cut -f 2- -d '.' | rev).res

FZN=$(echo $MZN | rev | cut -f 2- -d '.' | rev).fzn

mzn2fzn -Ggecode -o $FZN $MZN

# Compress xml and mzn files
lzma ${T%.*}
lzma $FZN 

