/*
    alexjlz_hash.c
    created at 2015/4/11
    last modified at 2015/4/11
    hash function for alexjlz
*/

#include <string.h>
const static char asctable[] = 
{
    [0] = '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

int alexjlz_hash(char *buff, char *hash) // len(hash) should >= 20
{
    char *p = buff;
    int tmp = 0;
    int index = 0;

    while( *p != 0 )
    {
        tmp = (*p + *(p+1) + hash[index])%16;
        hash[index] = asctable[tmp];
        index = (index+1)%20;
        p++;
    }

    return 0;
}
