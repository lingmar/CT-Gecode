#!/bin/sh

#find . -name "*.res" | parallel -P 4 --progress nice ./processone.sh {}

REG="reg.log"
TUP="tup.log"
CT="ct.log"

touch $REG
touch $TUP
touch $CT

cat out.log | grep "reg" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $REG
cat out.log | grep "tup" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $TUP
cat out.log | grep "ct" | cut -d "(" -f2 | cut -d ")" -f1 | sort -n > $CT

for f in $REG $TUP $CT; do
    count=1
    for line in `cat $f`; do
        echo "$line $count" >> "$(echo $f | cut -d '.' -f1).data"
        let "count+=1"
    done
done
