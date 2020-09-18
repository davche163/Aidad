#!/bin/sh
if [ $# -eq 0 ];
then
    echo "usage: ./XXX.sh SN"
    exit
fi      
if [ -n "$1" ]; then
    SN=$1
else
    SN=100001
fi
killall guard

#sleep 35
/home/openailab/process/guard/guard -t device-topic -h mqtt.dms.aodianyun.com -p 1883 -u pub_f95f8eaeb86fbb2c8bec24d251c3c453 -P sub_9cc92e5fd348968e55a702d7cd1ba475 -i ${SN} > /home/openailab/process/guard/log.txt &
