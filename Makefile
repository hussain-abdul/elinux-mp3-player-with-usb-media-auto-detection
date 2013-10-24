CC=gcc

CFLAGS = -I /usr/include/gstreamer-0.10/ -I /usr/include/glib-2.0/ -I /usr/include/libxml2/ -I /usr/lib/arm-linux-gnueabihf/glib-2.0/include/

LIBS = -L /usr/lib/arm-linux-gnueabihf/ -lpthread -lgstreamer-0.10 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lrt -lxml2 -lglib-2.0 -ludev

OBJS=player.o media_mount.o media_search.o media_play.o

all:player

player:${OBJS}
	${CC} ${OBJS} -o $@ ${LIBS}
  
%.o:%.c
	${CC} -c $< -o $@ ${CFLAGS}	

.PHONY = clean

clean:
	-rm player
	-rm player.o
	-rm media_mount.o
	-rm media_search.o
	-rm media_play.o
	rm *~
