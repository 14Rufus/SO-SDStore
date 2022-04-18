CC = gcc
CFLAGS = -Wall -g -I$(INCDIR)
OBJDIR = obj
SRCDIR = src
INCDIR = includes
#OBJS = $(OBJDIR)/auxs.o $(OBJDIR)/task.o
#SERVER_OBJS = $(OBJDIR)/aurrasd.o $(OBJDIR)/server_child.o
#CLIENT_OBJS = $(OBJDIR)/aurras.o


all: server client
		mkfifo bin/client_server_pipe
		mkfifo bin/server_client_pipe 

server: bin/aurrasd

client: bin/aurras

bin/aurrasd: obj/aurrasd.o
				gcc -g obj/aurrasd.o -o bin/aurrasd

obj/aurrasd.o: src/aurrasd.c
				gcc -Wall -g -c src/aurrasd.c -o obj/aurrasd.o

bin/aurras: obj/aurras.o
				gcc -g obj/aurras.o -o bin/aurras

obj/aurras.o: src/aurras.c
				gcc -Wall -g -c src/aurras.c -o obj/aurras.o

clean:
	rm -f obj/* tmp/* bin/{aurras,aurrasd} 
	rm -f bin/client_server_pipe
	rm -f bin/server_client_pipe


test:
	bin/aurras samples/sample-1.mp3 tmp/sample-1.mp3
	bin/aurras samples/sample-2.mp3 tmp/sample-2.mp3

#clean:
#	rm -f $(OBJDIR)/*.o
#	rm -f aurras
#	rm -f aurrasd
#	rm -f server_client_pipe
#	rm -f client_server_pipe
#	rm -f error.txt
# 	rm -f log.txt
#	rm -f log.idx