#!/bin/sh

# Convert many files from mzn to fzn

find instances/pigeonsplus -name "*.mzn.lzma" | tail -n +$1 | head -n $2 | parallel --gnu -P 4 --progress nice  ./script_mzn2fzn_one.sh {}

      
   

