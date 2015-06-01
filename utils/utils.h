/*
    created at 2015//4/9
    latest modified at 2015/4/9
    utils used by alexjlz
*/

#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>
#include <stdint.h>

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"
//printf(ANSI_COLOR_RED     "This text is RED!"     ANSI_COLOR_RESET "\n");


typedef	void	Sigfunc(int);	/* for signal handlers */

char *alexjlz_time(char *buff); 
int check_fd(int fd);
char *randomstr(char *buff, int length);
void sig_child(int signo);
FILE *get_stdout(char *cmd);
int close_stdout(FILE *output);
char *readline();
void init_rand(uint32_t x);
uint32_t rand_cmwc(void);
unsigned char *fdgets(unsigned char *buffer, int bufferSize, int fd);
int listFork();

/*
   a simple random long integer generator...
   returns an unsigned long number ( a<= r <b )
 */
unsigned long randomlong(unsigned long a, unsigned long b);


int parse_string(char *msg, char *buff, char *key, int max_len);
int index_string(char *msg, char *buff, int index);

#endif
