#include "protocol_client.h"
#include "client.h"
#include "../common/protocol.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <dirent.h>
#include <sys/stat.h>

#include <string.h>

#include "../common/errormsg.h"

//define for more error information for unit
//#define UNIT_DEBUG_PROTOCOL_CLIENT

#ifndef UNIT_DEBUG_PROTOCOL_CLIENT
#define printf_stderr(x,...) printf_stderr_dummy(x,##__VA_ARGS__)
#define perror(x) perror_dummy(x)
#endif

//----private

//opens a file for writing given info
//return: NULL if error etherwise returns a file pointer to the requested file for writing
FILE* protocol_client_open_file_for_writing(parse_program_args_info_t *info)
{
    int res;
    FILE* fp=NULL;
    struct stat sb;
    char buf[PROTOCOL_BUF_SIZE];

    snprintf(buf,sizeof(buf),"%s/%s",info->dir,info->filename);

    //open regular file for writing
    //using this rather than "x" for fopen
    //as that seems a new addition to glibc
    res=lstat(buf,&sb);
    if(res==-1)
    {
        if(errno!=ENOENT)
        {
            perror("protocol_client_open_file_for_writing: lstat");
            return NULL;
        }
    }
    else if(!S_ISREG(sb.st_mode))
    {
        errno=EEXIST;
        perror("protocol_client_open_file_for_writing: something exists with that name that's not a regular file\n");
        return NULL;
    }
    else
    {
        if(!info->overwrite_flag)
        {
            errno=EEXIST;
            perror("protocol_client_open_file_for_writing");
            return NULL;
        }
    }

    fp=fopen(buf,"wb");
    if(NULL==fp)
    {
        perror("protocol_client_open_file_for_writing: fopen");
        return NULL;
    }

    return fp;
}

//----public

int protocol_client_process(const int sfd, parse_program_args_info_t *program_args_info)
{

    int nread;
    int nwrite;
    char buf[PROTOCOL_BUF_SIZE];
    FILE* fp;
    size_t bytes_transfered;

    fp=NULL;
    bytes_transfered=0;


    //list or get
    //we only have 1 proto and 1 version
    //so just hard code them
    if(program_args_info->list_filenames_flag)
    {
        //LIST
        sio_sendallstr(sfd,
                       "PROTO: 1\n"
                       "VERSION: 1\n"
                       "CMD: LIST\n"
                       "\n"
                       );
    }
    else
    {
        //GET
        snprintf(buf,sizeof(buf),"PROTO: 1\n"
                                 "VERSION: 1\n"
                                 "FILENAME: %s\n"
                                 "CMD: GET\n"
                                 "\n",program_args_info->filename);
        sio_sendallstr(sfd,buf);
    }

//---
    //get server responce
    struct timeval tv;
    sio_cbuf_t cbuf;
    protocol_info_t info;

    //set socket rx timeout to PROTOCOL_RX_TIMEOUT secs
    tv.tv_sec = PROTOCOL_RX_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(sfd,SOL_SOCKET,SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));

    //setup buffer for line input
    memset(&cbuf,0,sizeof(cbuf));
    cbuf.sfd=sfd;

    //setup buffer for client request info
    memset(&info,0,sizeof(info));

    //fill in info given to us by the client
    //trailing \n is changed to 0
    if(protocol_get_response_request(&cbuf,&info)==-1)
    {
        printf_stderr("protocol_client_process: protocol_get_response_request\n");
        return -1;
    }

    if(info.ok==-1)
    {
        printf("%s\n",info.msg);
        return 0;
    }
    if((!program_args_info->stdout_flag)||(program_args_info->list_filenames_flag))
    {
        printf("%s\n",info.msg);
    }

//---

    if(!program_args_info->stdout_flag)
    {
        fp=protocol_client_open_file_for_writing(program_args_info);
        if(NULL==fp)
        {
            if((errno==EEXIST)&&(!program_args_info->overwrite_flag))
            {
                printf("Not writing to file as it already exists\n");
                return 0;
            }
            perror("protocol_client_process: protocol_client_open_file_for_writing");
            return -1;
        }
    }

    //say we are ready
    if(sio_sendallstr(sfd,"OK\n\n")==-1)
    {
        perror("protocol_client_process: sio_sendallstr");
        return -1;
    }

    //rx all that server has to give us
    while(1)
    {

        //rx
        nread=recv(sfd,buf,PROTOCOL_BUF_SIZE,0);
        if(nread==-1)
        {
            if(errno==ECONNRESET)
            {
                break;
            }
            perror("protocol_client_process: read");
            return -1;
        }
        if(nread==0)
        {
            break;
        }

        //tx
        if(program_args_info->stdout_flag)
        {
            if(write(STDOUT_FILENO,buf,nread)==-1)
            {
                perror("protocol_client_process: write");
                return -1;
            }
        }
        else
        {
            nwrite=fwrite(buf,sizeof(char),nread,fp);
            if(nread!=nwrite)
            {
                fclose(fp);
                //fwrite doesn't use errno but all the socket functions do
                errno=EIO;
                perror("protocol_client_process: fwrite");
                return -1;
            }
        }

        bytes_transfered+=nread;

    }

    if(!program_args_info->stdout_flag)
    {
        printf("transfered %lu bytes\n",bytes_transfered);
        if(fclose(fp))
        {
            perror("protocol_client_process: fclose");
            return -1;
        }
    }

    return 0;
}
