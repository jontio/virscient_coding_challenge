#include "parse_program_args.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int parse_program_args(int argc, char *argv[], parse_program_args_info_t *info)
{

    info->overwrite_flag=0;
    info->stdout_flag=0;
    info->list_filenames_flag=0;
    info->filename=NULL;
    info->dir=PARSE_PROGRAM_ARGS_DEFAULT_DIRECTORY;
    info->server_port=PARSE_PROGRAM_ARGS_DEFAULT_SERVER_PORT;
    info->server_host=PARSE_PROGRAM_ARGS_DEFAULT_SERVER_HOST_ADDRESS;

    int c;

    opterr = 0;

    if(
       (argc<=1)||
       (argc==2&&(strcmp(argv[1],"?")==0))
            )
    {
        printf("Usage [-h host] [-p port] [-d directory] [-s] [-l] [-t] [-f filename].\n");
        printf("-h host: server host address. (default is %s)\n",info->server_host);
        printf("-p port: server host port. (default is %s)\n",info->server_port);
        printf("-d directory: path where files are saved. (default is \'%s\')\n",info->dir);
        printf("-s: output file to stdio\n");
        printf("-l: list files on server\n");
        printf("-t: write over existing files\n");
        printf("-f filename: get file from server called filename\n");
        return -1;
    }

    while((c=getopt(argc,argv,"sltf:h:p:d:"))!=-1)
    {
        switch(c)
        {
        case 's':
            info->stdout_flag=1;
            break;
        case 'l':
            info->list_filenames_flag = 1;
            break;
        case 't':
            info->overwrite_flag = 1;
            break;
        case 'f':
            info->filename=optarg;
            break;
        case 'h':
            info->server_host=optarg;
            break;
        case 'p':
            info->server_port=optarg;
            break;
        case 'd':
            info->dir=optarg;
            break;
        case '?':
            if(optopt=='h'||optopt=='p'||optopt=='f'||optopt=='d')
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

    if(!info->list_filenames_flag&&info->filename==NULL)
    {
        fprintf(stderr,"Filename option needed when requesting a file\n");
        return -1;
    }

    if(info->list_filenames_flag&&info->filename!=NULL)
    {
        fprintf(stderr,"Filename option ignored when listing files; printing to STDOUT\n");
    }

    if(info->list_filenames_flag)
    {
        info->stdout_flag=1;
    }

    return 0;
}
