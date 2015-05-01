#include "prime.h"

int isprime(unsigned long long n) 
{
    unsigned long i = 0;
    if (n <= 3) {
        return n > 1;
    }

    if (n % 2 == 0 || n % 3 == 0) {
        return 0;
    }

    for (i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) {
            return 0;
        }
    }

    return 1;
}

unsigned long long getprime(int n)
{
    unsigned long long r = 5;
    unsigned long long s = 0;

    while ( s < n )
    {
        if ( r % 6 == 5 )
            r = r + 2;
        else
            r = r + 6;

        if ( isprime(r) )
            s++;
    }

    return r;
}

unsigned long long simple_getprime(int n)
{
    unsigned long long r = 5;
    unsigned long long s = 0;

    while ( s < n )
    {
        r = r + 6;
        if ( isprime(r) )
            s++;
    }

    return r;
}
