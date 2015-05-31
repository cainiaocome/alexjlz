/*
    core.h

    last modified at: 2015/5/28
    
    defines packet and functions used in communicate between client and server
*/
#ifndef _CORE_H
#define _CORE_H

#include <stdio.h>
#include <stdint.h>
#include "../adt/list.h"

struct client
{
    char uuid[256];
    char ip[32];
    char info[512];
    char task[1024];
};

struct packet
{
    uint32_t type;
    uint32_t flag;
    char value[1024];
};

/*
    cmd
    arg list
*/
struct alexjlz_packet
{
    char value[1024];
};

int alexjlz_register(struct client *c);
void *serve_client( void *arg);
int ask_for_service( int server_fd );

void *serve_alexjlz( void *arg);
int process_command(struct alexjlz_packet *p, list_p output);

/*
    packet struct design:
    the most significant four bits represent packet type
*/

#define packet_register                 ( 0x1 )
#define packet_task_sign                     ( 0x2 )

#endif
