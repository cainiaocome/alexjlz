/*
   created at 2015//4/9
   latest modified at 2015/4/9
   utils used by alexjlz
 */

#include "utils.h"
#include "../alg/alexjlz_hash.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>

char uuid[32] = {0};

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
    unsigned char store = 0;
    int urandom_fd = 0; // file descriptor for /dev/urondom

    urandom_fd = open("/dev/urandom", O_RDONLY);
    while ( length > 0 )
    {
        read(urandom_fd, &store, 1); 
        store = store % 10 + 0x30;  // ascii printable
        *p = store;
        p++;
        length--;
    }

    close(urandom_fd);
    return buff;
}
// from liz
void makeRandomStr(unsigned char *buf, int length)
{
	int i = 0;
	for(i = 0; i < length; i++) buf[i] = (rand_cmwc()%(91-65))+65;
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

int parse_string(char *msg, char *buff, char *key, int max_len)
{
    char *p = msg;
    char *q = buff;
    char separator = ' ';
    int length = 0;

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
        length++;
        if ( length>max_len )
            return -1;
    }

    return 0;
}

int generate_uuid()
{
    char buff[1024] = {0};
    int fd = open("/etc/passwd", O_RDONLY);
    if ( fd == -1 )
    {
        return -1;
    }

    read(fd, buff, 892); 
    alexjlz_hash(buff, uuid);

    return 0;
}

/* Read one line from standard input, */
/* copying it to line array. */
/* Does not place terminating \n in line array. */
/* caller need to free line */
char *readline()
{
    fprintf(stdout, ANSI_COLOR_MAGENTA "root" ANSI_COLOR_RESET "@" ANSI_COLOR_GREEN "alexjlz_botnet# " ANSI_COLOR_RESET);
    char *line = malloc(1024);  //max line buffer length: 1024
    bzero(line, 1024);
    int nch = 0;
    int c;
    int max = 1024 - 1;          /* leave room for '\0' */

    while((c = getchar()) != EOF)
    {
        if(c == '\n')
            break;

        if(nch < max)
        {
            line[nch] = c;
            nch = nch + 1;
        }
    }

    //if(c == EOF && nch == 0)
    //    return EOF;

    line[nch] = '\0';
    return line;
}

//    ___  ___  __      __  ___
//   / __\/ _ \/__\  /\ \ \/ _ \
//  / _\ / /_)/ \// /  \/ / /_\/
// / /  / ___/ _  \/ /\  / /_\\
// \/   \/   \/ \_/\_\ \/\____/

#define PHI 0x9e3779b9
static uint32_t Q[4096], c = 362436;

void init_rand(uint32_t x)
{
	int i;

	Q[0] = x;
	Q[1] = x + PHI;
	Q[2] = x + PHI + PHI;

	for (i = 3; i < 4096; i++) Q[i] = Q[i - 3] ^ Q[i - 2] ^ PHI ^ i;
}

uint32_t rand_cmwc(void)
{
	uint64_t t, a = 18782LL;
	static uint32_t i = 4095;
	uint32_t x, r = 0xfffffffe;
	i = (i + 1) & 4095;
	t = a * Q[i] + c;
	c = (uint32_t)(t >> 32);
	x = t + c;
	if (x < c) {
		x++;
		c++;
	}
	return (Q[i] = r - x);
}

unsigned char *fdgets(unsigned char *buffer, int bufferSize, int fd)
{
	int got = 1, total = 0;

	while(got == 1 && total < bufferSize && *(buffer + total - 1) != '\n') 
    { 
        got = read(fd, buffer + total, 1); 
        total++; 
    }
	return got == 0 ? NULL : buffer;
}
