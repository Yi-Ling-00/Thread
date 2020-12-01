TARGET = server deliver
DEPS = packet.h global.h clientSession.h
CFLAGS = -lpthread

all:  ${TARGET}

server: server.c ${DEPS}
	gcc -o server server.c ${CFLAGS}

deliver: deliver.c ${DEPS}
	gcc -o deliver deliver.c ${CFLAGS}

clean:
	rm -f ${TARGET}