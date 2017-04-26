#!/bin/sh

find . -name "*.res" | parallel -P 16 --progress nice ./processone.sh {}

REG="reg.log"
TUP_SPEED="tup_speed.log"
TUP_MEM="tup_mem.log"
CT="ct.log"

touch $REG
touch $TUP_MEM
touch $TUP_SPEED
touch $CT

cat out.log | grep "reg" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $REG
cat out.log | grep "tup_mem" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $TUP_MEM
cat out.log | grep "tup_speed" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $TUP_SPEED
cat out.log | grep "ct" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $CT

for f in $REG $TUP_MEM $TUP_SPEED $CT; do
    count=1
    for line in `cat $f`; do
        echo "$line $count" >> "$(echo $f | cut -d '.' -f1).data"
        let "count+=1"
    done
done
