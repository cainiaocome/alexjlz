/*
    log.c

    last modified at: 2015/4/18
    
    it finally works !!!
    
    defines log utils
*/

#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>  // variable argument for alexjlz_log

const char *log_file_path = "/tmp/alexjlz_log";

int alexjlz_log(char *format, ... )
{
    int log_fd = 0;
    char msg[1024] = {0};
    va_list a_list;
    va_list b_list;

    log_fd = open(log_file_path, O_RDWR | O_CREAT | O_APPEND);
    if(log_fd == -1)
    {
        perror("open");
        return -1;
    }

    // http://stackoverflow.com/questions/7031116/how-to-create-function-like-printf-variable-argument/7031174
    // http://www.cprogramming.com/tutorial/lesson17.html
    va_start(a_list, format); 
    va_copy(b_list, a_list);

    vsprintf(msg, format, b_list);
    write(log_fd, msg, strlen(msg));

    va_end(a_list);
    va_end(b_list);

    close(log_fd);

    return 0;
}
