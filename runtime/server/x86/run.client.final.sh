#!/bin/bash

#for x in  1 2  3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 25 30 35 40;
for x in  1;
do
    rm -rf clent.${x}.log
    bash run.client.sh ${x} clent.${x}.log
    response_time=`cat clent.${x}.log | grep "websocket_client\.cc\:74" | awk '{if($NF < 100){sum += $NF; n += 1}};END{print sum/n}'`
    a=`cat server.log | grep "websocket_server\.cc\:90" | awk '{if($NF<1000){sum += $NF; n+=1}};END{print sum/n;}'`
    echo ${x} ${response_time} ${a}
done