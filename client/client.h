#ifndef CLIENT_H
#define CLIENT_H

#include "../common/sio.h"

//name: host to connect to eg 192.168.0.1 etc
//service: either port eg "12345", or a name like "ftp"
//return: -1 on error else 0
int client_connect(const char *name, const char *service, struct addrinfo *addr);

//return: -1 on error else 0
int client_disconnect(int sfd);

#endif

