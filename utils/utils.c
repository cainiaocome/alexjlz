/*
    created at 2015//4/9
    latest modified at 2015/4/9
    utils used by alexjlz
*/

#include "utils.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

char *alexjlz_time(char *buff)
{
    time_t count_since_epoch = 0;
    char * ascii_time = NULL;

    count_since_epoch = time(NULL);
    if (count_since_epoch == -1)
    {
        perror("time");
        return NULL;
    }

    ascii_time = (char *)ctime( &count_since_epoch );
    if (ascii_time == NULL)
    {
        perror("ctime");
        return NULL;
    }
    
    snprintf(buff, 1024, "%s", ascii_time);
    // free ascii_time will generate error
    /*
    if ( ascii_time != NULL)
    {
        free(ascii_time);
        ascii_time = NULL;
    }
    */
    return buff;
}

int check_fd(int fd)
{
    return fcntl(fd, F_GETFD) != -1 || errno != EBADF;
    //return fcntl(fd, F_GETFD) != -1;
}

char *randomstr(char *buff, int length)
{
    char *p = buff;
    char store = 0;
    int urandom_fd = 0; // file descriptor for /dev/urondom

    urandom_fd = open("/dev/urandom", O_RDONLY);
    while ( length > 0 )
    {
        read(urandom_fd, &store, 1); 
        *p = store;
        p++;
        length--;
    }

    close(urandom_fd);
    return buff;
}