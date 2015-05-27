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
#include <signal.h>
#include <string.h>

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

/*
   return an unsigned long number ( a<= r <b )
 */
unsigned long randomlong(unsigned long a, unsigned long b)
{
    unsigned long distance = b>a?b-a:a-b;
    int urandom_fd = 0; // file descriptor for /dev/urondom
    unsigned long r = 0;

    urandom_fd = open("/dev/urandom", O_RDONLY);

    read(urandom_fd, &r, sizeof(r)); 

    close(urandom_fd);
    return r%distance + a;
}


void sig_child(int signo)
{
	pid_t	pid;
	int		stat;

	while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
		//printf("child %d terminated\n", pid);
        ;
	return;
}

Sigfunc *
signal(int signo, Sigfunc *func)
{
	struct sigaction	act, oact;

	act.sa_handler = func;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (signo == SIGALRM) {
#ifdef	SA_INTERRUPT
		act.sa_flags |= SA_INTERRUPT;	/* SunOS 4.x */
#endif
	} else {
#ifdef	SA_RESTART
		act.sa_flags |= SA_RESTART;		/* SVR4, 44BSD */
#endif
	}
	if (sigaction(signo, &act, &oact) < 0)
		return(SIG_ERR);
	return(oact.sa_handler);
}
/* end signal */

Sigfunc *
Signal(int signo, Sigfunc *func)	/* for our signal() function */
{
	Sigfunc	*sigfunc;

	if ( (sigfunc = signal(signo, func)) == SIG_ERR)
    {
		perror("signal error");
        exit(-1);
    }
	return(sigfunc);
}

FILE *get_stdout(char *cmd)
{
    FILE *output = popen(cmd, "r");

    return output;
}
int close_stdout(FILE *output)
{
    if (output != NULL)
    {
        pclose(output);
    }

    return 0;
}

int index_string(char *msg, char *buff, int index)
{
    char *p = msg;
    char *q = buff;
    char *r = msg;
    char separator = ' ';
    int this_index = 0;

    while ( this_index < index )
    {
        r = strchr(r, separator);
        if ( r == NULL )
        {
            return -1;
        }
        r++;
        this_index++;
    }
    while ( *r != separator && *r != '\0' )
    {
        *q = *r;
        q++;
        r++;
    }

    return 0;
}

int parse_string(char *msg, char *buff, char *key)
{
    char *p = msg;
    char *q = buff;
    char separator = ' ';

    p = strstr(msg, key);
    if ( p == NULL )
    {
        return -1;
    }
    p = p + strlen(key) + 1;
    while ( *p != separator && *p != '\0' )
    {
        *q = *p;
        q++;
        p++;
    }

    return 0;
}

