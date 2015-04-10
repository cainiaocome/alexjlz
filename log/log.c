/*
    log.c

    last modified at: 2015/4/10
    
    defines log utils
*/

#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

const char *log_file_path = "/tmp/alexjlz_log";

int alexjlz_log(char *msg)
{
    int log_fd = 0;

    log_fd = open(log_file_path, O_RDWR | O_CREAT | O_APPEND);
    if(log_fd == -1)
    {
        perror("open");
        return -1;
    }
    write(log_fd, msg, strlen(msg));

    close(log_fd);

    return 0;
}
