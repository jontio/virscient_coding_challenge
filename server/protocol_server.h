#ifndef PROTOCOL_SERVER_H
#define PROTOCOL_SERVER_H

//to be called when a client connects
//and communication takes place with
//a defined communication protocol.
//csfd: client socket file descriptor
//path: path that files are served from
//return: -1 for error 0 else
int protocol_server_process(const int csfd, const char *path);

#endif
