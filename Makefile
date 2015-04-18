distclean : clean
	rm -rf server client
	kill 9 `pidof server`
	kill 9 `pidof client`
clean :
	rm -rf *.o *.a
	rm -rf utils/*.o utils/*.a
	rm -rf tcpip/*.o tcpip/*.a
	rm -rf core/*.o core/*.a
	rm -rf log/*.o log/*.a
	rm -rf alg/*.o alg/*.a

all: client server
	> /tmp/alexjlz_log
	kill 9 `pidof server`
	kill 9 `pidof client`

.PHONY : log
log : /tmp/alexjlz_log
	cat /tmp/alexjlz_log

server : server.o utils.a tcpip.a core.a log.a alexjlz_hash.a
	gcc -o server server.o ./utils/utils.a ./tcpip/tcpip.a ./core/core.a ./log/log.a ./alg/alexjlz_hash.a

server.o : server.c
	gcc -c server.c

client : client.o utils.a tcpip.a core.a log.a alexjlz_hash.a
	gcc -o client client.o ./utils/utils.a ./tcpip/tcpip.a ./core/core.a ./log/log.a ./alg/alexjlz_hash.a

client.o : client.c
	gcc -c client.c

utils.a : ./utils/utils.c ./utils/utils.h
	gcc -c -o ./utils/utils.o ./utils/utils.c
	ar -rc ./utils/utils.a ./utils/utils.o

tcpip.a : ./tcpip/tcpip.c ./tcpip/tcpip.h
	gcc -c -o ./tcpip/tcpip.o ./tcpip/tcpip.c
	ar -rc ./tcpip/tcpip.a ./tcpip/tcpip.o

core.a : ./core/core.h ./core/core.c utils.a
	gcc -c -o ./core/core.o ./core/core.c
	ar -rc ./core/core.a ./core/core.o ./utils/utils.o

log.a : ./log/log.c ./log/log.h
	gcc -c -o ./log/log.o ./log/log.c
	ar -rc ./log/log.a ./log/log.o

alexjlz_hash.a : ./alg/alexjlz_hash.c ./alg/alexjlz_hash.h
	gcc -c -o ./alg/alexjlz_hash.o ./alg/alexjlz_hash.c
	ar -rc ./alg/alexjlz_hash.a ./alg/alexjlz_hash.o
