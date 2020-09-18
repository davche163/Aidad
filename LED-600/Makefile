export PKG_CONFIG_PATH=/usr/local/AID/pkgconfig:$PKG_CONFIG_PATH

LDDFLAGS= -lrtspclient -lrockchip_rga -lrockchip_mpp -lcam_engine_cifisp -lrockchip_vendor -ldrm -lcam_ia -lpthread  `pkg-config --libs bladecv` `pkg-config --libs gtk+-3.0` `pkg-config --libs tengine`
INCFLAGS= `pkg-config --cflags bladecv` `pkg-config --cflags gtk+-3.0` `pkg-config --cflags tengine`

all:
	g++ -g -c led-rtsp.cpp ipc_rtsp.cpp ledssd.cpp common.cpp  ${INCFLAGS}
	g++ led-rtsp.o ipc_rtsp.o ledssd.o common.o ${LDDFLAGS} -o led-rtsp
