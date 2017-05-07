#!/bin/bash

for f in `find . -name "*.data"`; do
    count=0
    for line in `cat $f | cut -d . -f1`; do

        if (( $line > 1000000 ))
        then
            count=$(($count+1))
        fi
    done
    echo "$f: $count timeouts"
done
