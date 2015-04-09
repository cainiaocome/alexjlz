clean :
	rm -rf *.o *.a
	rm -rf utils/*.o utils/*.a
	rm -rf tcpip/*.o tcpip/*.a

all_clean : clean
	rm -rf server client

all: client server

server : server.o utils.a tcpip.a
	gcc -o server server.o ./utils/utils.a ./tcpip/tcpip.a

server.o : server.c
	gcc -c server.c

client : client.o utils.a tcpip.a
	gcc -o client client.o ./utils/utils.a ./tcpip/tcpip.a

client.o : client.c
	gcc -c client.c

utils.a : ./utils/utils.c ./utils/utils.h
	gcc -c -o ./utils/utils.o ./utils/utils.c
	ar -rc ./utils/utils.a ./utils/utils.o

tcpip.a : ./tcpip/tcpip.c ./tcpip/tcpip.h
	gcc -c -o ./tcpip/tcpip.o ./tcpip/tcpip.c
	ar -rc ./tcpip/tcpip.a ./tcpip/tcpip.o
