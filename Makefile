VPATH = adt core log alg daemon tcpip utils tools
CC = gcc
CC_FLAGS = -pthread -static
private_libs = utils.a tcpip.a core.a log.a daemon.a libds.a alg.a

distclean : clean
	rm -rf server client
	> /tmp/alexjlz_log
	kill 9 `pidof server`
	kill 9 `pidof client`
	kill 9 `pidof alexjlz`
	kill 9 `pidof selinux`
clean :
	rm -rf *.o *.a client server alexjlz client.h alexjlz.h update.h
	cd adt; make clean
	cd alg; make clean
	cd tools; make clean
	cd weapons; make clean

all:
	> /tmp/alexjlz_log
	
	cd tools; make
	cd adt; make
	cd alg; make
	./tools/alexxxd -i update update.h
	cd weapons; make
	make client
	make alexjlz
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

alexjlz.o : alexjlz.c

alexjlz : alexjlz.o $(private_libs)
	$(CC) $(CC_FLAGS) -o alexjlz $^

%.o : %.c %.h
	$(CC) $(CC_FLAGS) -c -o $@ $<
%.a : %.o
	ar -rc $@ $<
%:%.sh
