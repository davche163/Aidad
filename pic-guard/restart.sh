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
/usr/bin/python2.7 /home/openailab/process/pic-guard/guard-upload.py -s ${SN} -l 3 &

echo done
