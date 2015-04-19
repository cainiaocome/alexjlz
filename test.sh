#!/usr/bin/env bash

declare -i count=1
while [ 1 -lt 2 ]
do
    echo $count
    count=$(( $count + 1 ))
    ./client &
done
