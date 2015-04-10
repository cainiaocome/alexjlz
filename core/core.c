/*
    core.c

    last modified at: 2015/4/10
    
    defines packet and functions used in communicate between client and server
*/

#include "core.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct packet *make_packet(unsigned long t, unsigned long l, char *v, struct packet *p)
{
    if (l > 1024)
    {
        perror("make_packet: packet length too long");
        exit(-1);
    }

    bzero(p, sizeof(p));
    p->type = t;
    p->length = l;
    strncpy((char*)&(p->value), (const char*)v, l);
    
    return p;
}

struct packet *parse_packet(struct packet *p)
{
    unsigned long type = p->type;
    unsigned long length = p->length;
    char *buff = (char *)&(p->value);

    printf("packet type: %d\n", type);
    printf("packet length: %d\n", length);
    printf("packet value:%s\n", buff);

    return p;
}
