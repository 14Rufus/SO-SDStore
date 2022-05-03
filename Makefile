#########################################################################################################
CC = gcc 
CFLAGS = -Wall -g -c -o
CFLAGS1 = -g -o
MKDIR = @mkdir -p
#########################################################################################################



#########################################################################################################


all: server client

server: bin/sdstored

client: bin/sdstore

bin/sdstored: obj/sdstored.o
	${CC} ${CFLAGS1} $@ $^

obj/sdstored.o: src/sdstored.c
	${CC} ${CFLAGS} $@ $^

bin/sdstore: obj/sdstore.o
	${CC} ${CFLAGS1} $@ $^

obj/sdstore.o: src/sdstore.c
	${CC} ${CFLAGS} $@ $^

setup:
	${MKDIR} obj
	${MKDIR} bin
	${MKDIR} tmp

clean:
	@-rm obj/* tmp/* bin/sdstore bin/sdstored