clean :
	rm -rf *.o *.a
	rm -rf utils/*.o utils/*.a
	rm -rf tcpip/*.o tcpip/*.a
	rm -rf core/*.o core/*.a

all_clean : clean
	rm -rf server client

all: client server

server : server.o utils.a tcpip.a core.a log.a
	gcc -o server server.o ./utils/utils.a ./tcpip/tcpip.a ./core/core.a ./log/log.a

server.o : server.c
	gcc -c server.c

client : client.o utils.a tcpip.a core.a log.a
	gcc -o client client.o ./utils/utils.a ./tcpip/tcpip.a ./core/core.a ./log/log.a

client.o : client.c
	gcc -c client.c

utils.a : ./utils/utils.c ./utils/utils.h
	gcc -c -o ./utils/utils.o ./utils/utils.c
	ar -rc ./utils/utils.a ./utils/utils.o

tcpip.a : ./tcpip/tcpip.c ./tcpip/tcpip.h
	gcc -c -o ./tcpip/tcpip.o ./tcpip/tcpip.c
	ar -rc ./tcpip/tcpip.a ./tcpip/tcpip.o

core.a : ./core/core.h ./core/core.c
	gcc -c -o ./core/core.o ./core/core.c
	ar -rc ./core/core.a ./core/core.o

log.a : ./log/log.c ./log/log.h
	gcc -c -o ./log/log.o ./log/log.c
	ar -rc ./log/log.a ./log/log.o
