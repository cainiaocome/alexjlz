/*
    daemon.h
    created at 2015/4/11
    last modified at 2015/4/11
    daemonize a process
    
    refer to http://www.linuxprofilm.com/articles/linux-daemon-howto.html for more
*/

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int daemonize();
