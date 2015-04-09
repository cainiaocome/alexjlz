/*
   created at 2015//4/9
   latest modified at 2015/4/9
   network functions used by alexjlz
 */

// create a listen socket, with bind to (inaddr_any, port)
int create_tcp_server(short port);
int connect_tcp_server(char *ip, short port);
