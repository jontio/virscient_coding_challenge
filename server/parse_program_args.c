#include "parse_program_args.h"


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int parse_program_args(int argc, char *argv[], parse_program_args_info_t *info)
{

    info->dir=PARSE_PROGRAM_ARGS_DEFAULT_DIRECTORY;
    info->server_port=PARSE_PROGRAM_ARGS_DEFAULT_SERVER_PORT;
    info->server_name=PARSE_PROGRAM_ARGS_DEFAULT_SERVER_NAME;
    info->max_nclients=PARSE_PROGRAM_ARGS_DEFAULT_MAX_NCLIENTS;

    int c;

    opterr = 0;

    if(
       (argc<=1)||
       (argc==2&&(strcmp(argv[1],"?")==0))
            )
    {
        printf("Usage [-n name] [-p port] [-d directory] [-m max_nclients].\n");
        if(info->server_name==NULL)
        {
            printf("-n name: server name. (default is ANY)\n");
        }
        else
        {
            printf("-n name: server name. (default is %s)\n",info->server_name);
        }
        printf("-p port: server port. (default is %s)\n",info->server_port);
        printf("-d directory: path where files are served from. (default is \'%s\')\n",info->dir);
        printf("-m max_nclients: max number of clients allowed. (default is %d)\n",info->max_nclients);
        return -1;
    }

    while((c=getopt(argc,argv,"n:p:d:m:"))!=-1)
    {
        switch(c)
        {
        case 'n':
            info->server_name=optarg;
            break;
        case 'p':
            info->server_port=optarg;
            break;
        case 'd':
            info->dir=optarg;
            break;
        case 'm':
            info->max_nclients=atoi(optarg);
            if(info->max_nclients<=0)
            {
                fprintf(stderr,"Option -m invalid.\n");
                return -1;
            }
            break;
        case '?':
            if(optopt=='n'||optopt=='p'||optopt=='d'||optopt=='m')
            {
                fprintf(stderr,"Option -%c requires an argument.\n",optopt);
            }
            else
            {
                fprintf(stderr,"Unknown option `-%c'.\n",optopt);
            }
            return -1;
        default:
            abort ();
        }
    }

    if(optind<argc)
    {
        fprintf(stderr,"Not expecting non option arguments\n");
        return -1;
    }

    return 0;
}
