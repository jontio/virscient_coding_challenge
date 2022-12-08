#ifndef PROTOCOL_CLIENT_H
#define PROTOCOL_CLIENT_H

#include "parse_program_args.h"

//prosess the client request given the server socket
//and parsed arguments
//return: -1 on error 0 else
int protocol_client_process(const int sfd, parse_program_args_info_t *program_args_info);

#endif
