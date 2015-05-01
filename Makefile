VPATH = adt core log alg daemon tcpip utils
distclean : clean
	rm -rf server client
	kill 9 `pidof server`
	kill 9 `pidof client`
clean :
	rm -rf *.o *.a

all: client server
	> /tmp/alexjlz_log
	kill 9 `pidof server`
	kill 9 `pidof client`

.PHONY : log
log : /tmp/alexjlz_log
	cat /tmp/alexjlz_log

server.o : server.c

server : server.o utils.a tcpip.a core.a log.a alexjlz_hash.a daemon.a
	gcc -o server $^

client.o : client.c

client : client.o utils.a tcpip.a core.a log.a alexjlz_hash.a
	gcc -o client $^

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

%.o : %.c %.h
	gcc -c -o $@ $<
%.a : %.o
	ar -rc $@ $<
