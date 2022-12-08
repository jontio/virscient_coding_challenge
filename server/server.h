#ifndef SERVER_H
#define SERVER_H

#include "../common/sio.h"

#ifndef SERVER_LISTEN_BACKLOG
#define SERVER_LISTEN_BACKLOG 10
#endif

typedef int (*protocol_process_ptr)(const int,const char *);

//name: NULL or something like 0.0.0.0 or 192.168.0.1 etc.
//service: either port eg "12345", or a name like "ftp".
//dir: path from where files are served from.
//max_nclients: max number of clients allowed at one time.
//protocol_process: function that server will call with the
//                  client socket descriptor as its argument.
//return: should never return. if an error happen then it will
//                  return -1 with errno set.
int server_start(const char *name, const char *service, const char *dir, int max_nclients, const protocol_process_ptr protocol_process);

#endif

