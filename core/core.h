/*
    core.h

    last modified at: 2015/5/28
    
    defines packet and functions used in communicate between client and server
*/
#ifndef _CORE_H
#define _CORE_H

#include "../adt/list.h"

#include <stdio.h>
#include <stdint.h>
#include <time.h>

#define OUTPUT_END ("!!!!!!")
#define USERNAME ("alexjlz")
#define PASSWORD ("alexsu")

struct client
{
    char uuid[256];
    char ip[32];
    char info[1024];
    char task[1024];
    char current_task[1024];
    char asc_last_heartbeat[64];
    time_t last_heartbeat;
    time_t task_starttime;
    time_t task_endtime;
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

int alexjlz_register(struct client *c, struct packet *p);
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
