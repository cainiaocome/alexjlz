CCFLAGS= -static
ps: ps.c
	gcc $(CCFLAGS) -o ps ps.c
test: test.c
	gcc -o test test.c
clean:
	rm -rf `ls | grep pscan` temp.* ps test
