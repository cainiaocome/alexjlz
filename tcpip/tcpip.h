/*
   created at 2015//4/9
   latest modified at 2015/4/9
   network functions used by alexjlz
 */

// create a listen socket, with bind to (inaddr_any, port)
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int getHost(unsigned char *toGet, struct in_addr *i);
in_addr_t getRandomPublicIP();
in_addr_t getRandomIP(in_addr_t netmask);
int create_tcp_server(short port);
int connect_tcp_server(char *ip, short port);
ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
int get_remote_ip(int fd, char remote_ip[]);


// from liz
unsigned short csum (unsigned short *buf, int count);
unsigned short tcpcsum(struct iphdr *iph, struct tcphdr *tcph);
void makeIPPacket(struct iphdr *iph, uint32_t dest, uint32_t source, uint8_t protocol, int packetSize);
void sendUDP(unsigned char *target, int port, int timeEnd, int spoofit, int packetsize, int pollinterval);
void sendTCP(unsigned char *target, int port, int timeEnd, int spoofit, unsigned char *flags, int packetsize, int pollinterval);
