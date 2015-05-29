#!/usr/bin/env bash

declare -i count=1
while [ 1 -lt 2 ]
do
    let clients=`ps aux | grep -c client`
    if [ $clients -gt 9 ];then
        sleep 4
    fi
    echo $count
    count=$(( $count + 1 ))
    ./client &
done
