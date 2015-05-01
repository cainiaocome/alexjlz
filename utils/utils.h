/*
    created at 2015//4/9
    latest modified at 2015/4/9
    utils used by alexjlz
*/

#include <stdio.h>

typedef	void	Sigfunc(int);	/* for signal handlers */

char *alexjlz_time(char *buff); 
int check_fd(int fd);
char *randomstr(char *buff, int length);
void sig_child(int signo);
FILE *get_stdout(char *cmd);
int close_stdout(FILE *output);

/*
   a simple random long integer generator...
   returns an unsigned long number ( a<= r <b )
 */
unsigned long randomlong(unsigned long a, unsigned long b);
