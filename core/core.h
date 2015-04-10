/*
    core.h

    last modified at: 2015/4/10
    
    defines packet and functions used in communicate between client and server
*/

struct packet
{
    unsigned long type;
    unsigned long length;
    char value[1024];
};

struct packet *make_packet(unsigned long, unsigned long, char*, struct packet*);
struct packet *parse_packet(struct packet *p);
