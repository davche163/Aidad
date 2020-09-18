#!/bin/bash

killall guard
mv guard guard.bak
export PKG_CONFIG_PATH=/usr/local/AID/pkgconfig:$PKG_CONFIG_PATH

g++ -std=c++11 -g -c -I /usr/local/include/ guard.cpp pubsub_opts.c ipc_rtsp.cpp `pkg-config --cflags bladecv` `pkg-config --cflags gtk+-3.0`
g++ -std=c++11 guard.o pubsub_opts.o ipc_rtsp.o -lpaho-mqtt3a -lcjson -lrtspclient -lpthread -lrockchip_rga -lrockchip_mpp -ldrm `pkg-config --libs bladecv` `pkg-config --libs gtk+-3.0` -o guard


# run
export LD_LIBRARY_PATH=/usr/local/lib64/:$LD_LIBRARY_PATH
#./guard -t device-topic -h mqtt.dms.aodianyun.com -p 1883 -u pub_f95f8eaeb86fbb2c8bec24d251c3c453 -P sub_9cc92e5fd348968e55a702d7cd1ba475 -k 100 &

