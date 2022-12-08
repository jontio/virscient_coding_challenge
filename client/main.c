#include "client.h"
#include "parse_program_args.h"
#include "protocol_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

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

    clock_t begin = clock();

    if(protocol_client_process(sfd,&info)==-1)
    {
        perror("main: protocol_client_process");
        client_disconnect(sfd);
        exit(EXIT_FAILURE);
    }

    clock_t end = clock();
    double time_spent = (double)(end - begin);

    if((!info.stdout_flag)&&(!info.list_filenames_flag))
    {
        printf("time taken %0.1fs\n",((double)(int)(time_spent/10000.0))/10.0);
    }

    client_disconnect(sfd);

    exit(EXIT_SUCCESS);

}
