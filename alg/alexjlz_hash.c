/*
    alexjlz_hash.c
    created at 2015/4/11
    last modified at 2015/4/11
    hash function for alexjlz
*/

#include <string.h>
#include <stdio.h>

unsigned long max_hash_buff_length = 1024;

unsigned long alexjlz_hash(char *buff)
{
    unsigned hash_result = 0;
    char *p = buff;

    if ( strlen(buff) > max_hash_buff_length)
    {
        return -1;
    }

    while( *p != 0 )
    {
        //fprintf(stdout, "hash_result: %lu\n", hash_result);
        hash_result =  *p + hash_result<<3;
        p++;
    }
    
    return hash_result;
}
