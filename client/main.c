#include "client.h"
#include "parse_program_args.h"
#include "protocol_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[])
{

    struct addrinfo addr;
    parse_program_args_info_t info;
    int sfd;
    int res;

    res=parse_program_args(argc,argv,&info);
    if(res<0)
    {
        exit(EXIT_FAILURE);
    }

    sfd=client_connect(info.server_host,info.server_port,&addr);
    if(sfd==-1)
    {
        perror("main: client_connect");
        exit(EXIT_FAILURE);
    }
    if(!info.stdout_flag)
    {
        sio_print_address("connected to",&addr);
    }

    if(protocol_client_process(sfd,&info)==-1)
    {
        perror("main: protocol_client_process");
        client_disconnect(sfd);
        exit(EXIT_FAILURE);
    }

    client_disconnect(sfd);

    exit(EXIT_SUCCESS);

}
