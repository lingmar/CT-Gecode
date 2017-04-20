#!/bin/sh

find . -name "*.xml.lzma" | tail -n +$1 | head -n $2 > "files.txt"

while read -r line
do 
    ./scriptone.sh $line &
    echo $line
done < "files.txt"

echo "Finish"

#find . -name "*.lzma" | tail  -n +$1 | head  -n $2 | parallel --gnu -P 8 --progress nice  ./scriptone.sh {}


      
   

