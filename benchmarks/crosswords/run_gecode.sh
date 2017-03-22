#!/bin/bash

i=0
while [ $i -lt 72 ]
do
    file="crossword3_$i.fzn"
    fzn-gecode -s $file > "solutions/gecode_$i.txt"
    i=$(($i + 1))
done
