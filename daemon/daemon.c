/*
    daemon.c
    created at 2015/4/11
    last modified at 2015/4/11
    daemonize a process
    
    refer to http://www.linuxprofilm.com/articles/linux-daemon-howto.html for more
*/

#include "../log/log.h"

#include "daemon.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int daemonize()
{
    pid_t pid, sid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
        return -1;
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* Change the file mode mask */
    umask(0);       

    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
        /* Log any failure here */
        return -1;
    }

    /* Change the current working directory */
    if ((chdir("/")) < 0) {
        /* Log any failure here */
        return -1;
    }

    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    return 0;
}
