/*
    log.c

    last modified at: 2015/4/18
    
    it finally works !!!
    
    defines log utils
*/

#include "log.h"
#include "../config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>  // variable argument for alexjlz_log

const char *log_file_path = LOG_FILE_PATH;

int alexjlz_log(int level, char *format, ... )
{
    int log_fd = 0;
    int lock = 0;
    char msg[1024] = {0};
    char time[64] = {0};
    va_list a_list;
    va_list b_list;

    if ( level > LOG_LEVEL )
    {
        return 0;
    }
    log_fd = open(log_file_path, O_RDWR | O_CREAT | O_APPEND);
    if(log_fd == -1)
    {
        return -1;
    }
    lock = flock(log_fd, LOCK_EX);  /* flock will not work under fork, but here it is ok */
    if(lock == -1)
    {
        return -1;
    }

    /*
    va_start(a_list, format); 
    va_copy(b_list, a_list);

    vsprintf(msg, format, b_list);
    write(log_fd, msg, strlen(msg));

    va_end(a_list);
    va_end(b_list);
    */
    // http://stackoverflow.com/questions/7031116/how-to-create-function-like-printf-variable-argument/7031174
    // http://www.cprogramming.com/tutorial/lesson17.html
    va_start(a_list, format); 

    vsprintf(msg, format, a_list);
    write(log_fd, msg, strlen(msg));

    va_end(a_list);

    lock = flock(log_fd, LOCK_UN);
    if(lock == -1)
    {
        return -1;
    }
    close(log_fd);

    return 0;
}
