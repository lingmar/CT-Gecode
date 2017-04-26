#!/bin/sh

# Run fzn-gecode for many files

find . -name "*.fzn" | tail -n +$1 | head -n +$2 | parallel --gnu -P 4 --progress nice ./scriptrunone.sh {}

./process.sh $1 $2





