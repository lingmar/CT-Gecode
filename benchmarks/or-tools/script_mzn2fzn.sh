#!/bin/sh

# Convert many files from mzn to fzn

find . -name "*.mzn.lzma" | tail -n +$1 | head -n $2 | parallel --gnu -P 4 --progress nice  ./script_mzn2fzn_one.sh {}

# while read -r line
# do 
#     ./script_mzn2fzn_one.sh $line
#     echo $line
# done < "files.txt"

# echo "Finish"

#find . -name "*.lzma" | tail  -n +$1 | head  -n $2 | parallel --gnu -P 8 --progress nice  ./scriptone.sh {}


      
   

