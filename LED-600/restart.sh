#!/bin/sh
killall led-rtsp
killall python
#move sleep to rc.local
#sleep 35
/home/openailab/process/LED-600/led-rtsp -m /home/openailab/process/LED-600/MobileNetSSD_LED_iter_40000_4tengine0.6.5.tmfile -l 10 &
