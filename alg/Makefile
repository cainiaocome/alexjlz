CFLAGS=-O2 -Wall
LDFLAGS=-L. -lds
HEADERS=alexjlz_hash.h gcd.h prime.h aes.h
OBJS=alexjlz_hash.o gcd.o prime.o aes.o
#PREFIX=/usr/local
CC=gcc

alg.a: $(OBJS)
	ar rcs alg.a $(OBJS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *test *.o *.a
