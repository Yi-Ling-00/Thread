TARGET = server deliver
DEPS = packet.h global.h clientSession.h
CFLAGS = -lpthread

all:  ${TARGET}

server: server.c ${DEPS}
	gcc -o server server.c ${CFLAGS}

deliver: client.c ${DEPS}
	gcc -o client client.c ${CFLAGS}

clean:
	rm -f ${TARGET}