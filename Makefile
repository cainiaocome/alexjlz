VPATH = adt core log alg daemon tcpip utils
CC = gcc
#CC_FLAGS = -static
CC_FLAGS = -pthread
private_libs = utils.a tcpip.a core.a log.a alexjlz_hash.a daemon.a libds.a

distclean : clean
	rm -rf server client
	kill 9 `pidof server`
	kill 9 `pidof client`
clean :
	rm -rf *.o *.a client server
	cd adt; make clean

all:
	> /tmp/alexjlz_log
	kill 9 `pidof server`
	kill 9 `pidof client`
	
	cd adt; make
	make client
	make server

.PHONY : log
log : /tmp/alexjlz_log
	cat /tmp/alexjlz_log

server.o : server.c

server : server.o $(private_libs)
	$(CC) $(CC_FLAGS) -o server $^

client.o : client.c

client : client.o $(private_libs)
	$(CC) $(CC_FLAGS) -o client $^

utils.o : utils.c utils.h
utils.a : utils.o
tcpip.o : tcpip.c tcpip.h
tcpip.a : tcpip.o
core.o : core.h core.c utils.a
core.a : core.o
daemon.o : daemon.c daemon.h
daemon.a : daemon.o
log.o : log.c log.h
log.a : log.o
alexjlz_hash.o : alexjlz_hash.c alexjlz_hash.h
alexjlz_hash.a : alexjlz_hash.o
dictionary.o : dictionary.c dictionary.h
dictionary.a : dictionary.o

%.o : %.c %.h
	$(CC) $(CC_FLAGS) -c -o $@ $<
%.a : %.o
	ar -rc $@ $<
%:%.sh
