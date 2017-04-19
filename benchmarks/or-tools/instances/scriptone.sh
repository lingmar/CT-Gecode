#!/bin/sh

# Script for converting xcsp2.1 to mz

D=`dirname $1`
D=$(echo $D | cut -f 2 -d '/')
T=$1
#du -h $1
unlzma $T
#du -h ${T%.*}
#echo $1
XML=${T%.*}
#echo $D $XML > $(echo $XML | rev | cut -f 2- -d '.' | rev).res

MZN=$(echo $XML | rev | cut -f 2- -d '.' | rev).mzn
touch $MZN
#echo $MZN

../../mzn2feat/xcsp2mzn/xcsp2mzn $XML > $MZN

# Compress xml and mzn files
lzma ${T%.*} &
lzma $MZN 

#echo "Finish"

