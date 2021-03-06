#!/bin/bash

board="$1"
shift

seed=46
found=0
while test "$found" = 0 ; do
    echo "Trying seed = $seed"
    if ./fc-solve --method random-dfs -sp r:tf -to '[0123456789ABCDE]' -seed "$seed" -mi 1000000 --freecells-num 3 "$board" | grep -E '^This game is solv' ; then
        found=1
        echo "Found seed = $seed"
    else
        let seed++
    fi
done
