/*
    created at 2015//4/9
    latest modified at 2015/4/9
    utils used by alexjlz
*/

typedef	void	Sigfunc(int);	/* for signal handlers */

char *alexjlz_time(char *buff); 
int check_fd(int fd);
char *randomstr(char *buff, int length);
void sig_child(int signo);
