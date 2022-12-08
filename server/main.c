#include "server.h"
#include "parse_program_args.h"
#include "protocol_server.h"

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    parse_program_args_info_t info;
    int res;

    res=parse_program_args(argc,argv,&info);
    if(res<0)
    {
        exit(EXIT_FAILURE);
    }

    server_start(info.server_name,info.server_port,info.dir,info.max_nclients,protocol_server_process);
    perror("server_start");
    return(EXIT_FAILURE);
}
