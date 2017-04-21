#!/bin/sh

# Run fzn-gecode for many files

find . -name "*.fzn.lzma" | tail -n +$1 | head -n +$2 | parallel --gnu -P 8 --progress nice  ./scriptrunone.sh {}
   
