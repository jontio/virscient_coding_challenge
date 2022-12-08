#include "protocol_server.h"
#include "server.h"
#include "../common/protocol.h"

#include <ctype.h>
#include <dirent.h>
#include <sys/stat.h>

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>

#include <errno.h>
#include <assert.h>

#include <math.h>

#include <time.h>

#include "../common/errormsg.h"

//define for more error information for unit
//#define UNIT_DEBUG_PROTOCOL_SERVER

#ifndef UNIT_DEBUG_PROTOCOL_SERVER
#define printf_stderr(x,...) printf_stderr_dummy(x,##__VA_ARGS__)
#define perror(x) perror_dummy(x)
#endif

//--------private

//check if regular file exists
//info: contains filename
//path: path where file is
//return: -1 for error else the size of the regular file in bytes
off_t protocol_server_check_regular_file_exists(const protocol_info_t *info, const char *path)
{
    struct stat sb;
    char buf[PROTOCOL_BUF_SIZE];

    if(snprintf(buf,sizeof(buf),"%s/%s",path,info->filename)>=(int)sizeof(buf))
    {
        perror("protocol_server_check_regular_file_exists: snprintf");
        return -1;
    }

    if(stat(buf,&sb)==-1)
    {
        perror("protocol_server_check_file_exists: stat");
        return -1;
    }

    if(!S_ISREG(sb.st_mode))
    {
        return -1;
    }

    return sb.st_size;

}

//sends list of files in a directory specified by path to the client
//csfd: client socket file descriptor
//path: path where file is
//return: on error returns -1 else returns 0
int protocol_server_send_file_list(int csfd, const char *path)
{
    char buf[PROTOCOL_BUF_SIZE];
    int buf_wptr;
    DIR *folder;
    struct dirent *entry;
    struct stat stat_info;
    char time_str[20];
    struct tm *timeinfo;
    char line_entry[PROTOCOL_BUF_SIZE-1];
    int x;

    //we have to have have enough room to fit a file name plus an extra \n
    assert(sizeof(buf)>=(sizeof(line_entry)+1));

    if(path==NULL)
    {
        folder=opendir(".");
    }
    else
    {
        folder=opendir(path);
    }
    if(folder==NULL)
    {
        perror("protocol_send_file_list: opendir");
        return -1;
    }

    buf_wptr=0;
    errno=0;
    while((entry=readdir(folder)))
    {
        if(!(entry->d_type==DT_REG||entry->d_type==DT_UNKNOWN))
        {
            continue;
        }

        //tmp use line_entry for path and name
        if(snprintf(line_entry,sizeof(line_entry),"%s/%s",path,entry->d_name)>=(int)sizeof(line_entry))
        {
            perror("protocol_send_file_list: snprintf");
            return -1;
        }

        //create a line containing size (bytes),last modified (local) and filename
        if(stat(line_entry,&stat_info)==-1)
        {
            perror("protocol_send_file_list: stat");
            closedir(folder);
            return -1;
        }
        timeinfo=localtime(&stat_info.st_mtime);
        strftime(time_str,sizeof(time_str),"%b %d %H:%M",timeinfo);
        if(stat_info.st_size>=10)
        {
            x=(int)log10(stat_info.st_size);
        }
        else
        {
            //if file size==0 then log10 is undefined
            x=0;
        }
        snprintf(line_entry,sizeof(line_entry),"%*s%ld\t%s\t%s",10-x,"",stat_info.st_size,time_str,entry->d_name);

        //flush if can't fit in another line_entry
        if((sizeof(buf)-buf_wptr)<(strlen(line_entry)+1))
        {
            if(buf_wptr==0)
            {
                //this should never happen because of the assert above
                printf_stderr("protocol_send_file_list: can't fit filename into name buffer");
                closedir(folder);
                return -1;
            }
            buf_wptr=0;
            if(sio_sendall(csfd,buf,strlen(buf))==-1)
            {
                perror("protocol_send_file_list: send");
                closedir(folder);
                return -1;
            }
        }

        //fill
        strcpy(buf+buf_wptr,line_entry);
        buf_wptr+=strlen(line_entry);
        buf[buf_wptr]='\n';
        buf_wptr++;
        buf[buf_wptr]=0;

    }
    if(entry==NULL&&errno)
    {
        perror("protocol_send_file_list: readdir");
        closedir(folder);
        return -1;
    }

    //flush the name buffer
    if(buf_wptr>0)
    {
        buf_wptr=0;
        if(sio_sendall(csfd,buf,strlen(buf))==-1)
        {
            perror("protocol_send_file_list: send");
            closedir(folder);
            return -1;
        }
    }

    if(closedir(folder)==-1)
    {
        perror("protocol_send_file_list: closedir");
        return -1;
    }

    return 0;

}

//sends a file to the client
//csfd: client socket file descriptor
//path: path where file is
//return: -1 if an error happened with errno set to the error
//        else returns 0 on success
int protocol_server_send_file(int csfd, const char *path, const protocol_info_t *info)
{
    FILE* fp=NULL;
    struct stat sb;
    char buf[PROTOCOL_BUF_SIZE];
    int bread;

    //tmp use buf for path and name
    if(snprintf(buf,sizeof(buf),"%s/%s",path,info->filename)>=(int)sizeof(buf))
    {
        perror("protocol_send_file: snprintf");
        return -1;
    }

    //open regular file for reading
    if(lstat(buf,&sb)==-1)
    {
        perror("protocol_send_file: lstat");
        return -1;
    }
    else if(S_ISREG(sb.st_mode))
    {
        fp=fopen(buf,"rb");
        if(NULL==fp)
        {
            perror("protocol_send_file: fopen");
            return -1;
        }
    }

    //if not a regular file
    if(fp==NULL)
    {
        errno=EACCES;
        perror("protocol_send_file: not a regular file");
        return -1;
    }

    clearerr(fp);
    do
    {
        bread=fread(buf,sizeof(char),sizeof(buf),fp);
        if(sio_sendall(csfd,buf,bread)==-1)
        {
            perror("protocol_send_file: send");
            return -1;
        }
    }
    while(bread>0);
    if(ferror(fp))
    {
        //fread doesn't use errno but all the socket functions do
        errno=EIO;
        perror("protocol_send_file: fread");
        fclose(fp);
        return -1;
    }

    if(fclose(fp))
    {
        perror("protocol_send_file: fclose");
        return -1;
    }

    return 0;

}

//--------public

int protocol_server_process(const int csfd, const char *path)
{

    char buf[PROTOCOL_BUF_SIZE];
    struct timeval tv;
    int res;
    off_t filesize;
    sio_cbuf_t cbuf;
    protocol_info_t info;

    //set socket rx timeout to PROTOCOL_RX_TIMEOUT secs
    tv.tv_sec = PROTOCOL_RX_TIMEOUT;
    tv.tv_usec = 0;
    setsockopt(csfd,SOL_SOCKET,SO_RCVTIMEO,(struct timeval *)&tv,sizeof(struct timeval));

    //setup buffer for line input
    memset(&cbuf,0,sizeof(cbuf));
    cbuf.sfd=csfd;

    //setup buffer for client request info
    memset(&info,0,sizeof(info));

    //fill in info given to us by the client
    //trailing \n is changed to 0
    if(protocol_get_response_request(&cbuf,&info)==-1)
    {
        printf_stderr("protocol_process: protocol_get_response_request\n");
        return -1;
    }

    //currently we only have version 1 of proto 1.
    //lets not tell the client why we don't fulfill
    //its request; no point in tell it something
    //that could be used to figure out how we work.
    if(info.proto!=1)
    {
        printf_stderr("protocol_process: unsupported proto\n");
        return -1;
    }
    if(info.version!=1)
    {
        printf_stderr("protocol_process: unsupported version\n");
        return -1;
    }

    //send responce header
    switch(info.cmd)
    {
    case PROTOCOL_INFO_CMD_LIST:
        res=protocol_create_message(buf,sizeof(buf),0,"Listing");
        break;
    case PROTOCOL_INFO_CMD_GET:
        if((filesize=protocol_server_check_regular_file_exists(&info,path))==-1)
        {
            res=protocol_create_message(buf,sizeof(buf),-1,"%s doesn't exist",info.filename);
        }
        else
        {
            res=protocol_create_message(buf,sizeof(buf),0,"Here is %s for you. It is %ld bytes",info.filename,filesize);
        }
        break;
    default:
        protocol_info_cmd_to_string(info.cmd,buf,sizeof(buf));
        printf_stderr("protocol_process: unknown client command \'%s\'\n",buf);
        return -1;
        break;
    }
    if(res==-1)
    {
        printf_stderr("protocol_process: protocol_create_message\n");
        return -1;
    }
    if(sio_sendallstr(csfd,buf)==-1)
    {
        printf_stderr("protocol_process: sio_sendallstr\n");
        return -1;
    }

    //wait for ok from client
    //this tell us they are ready
    //and avoids the protocol_get_response_request
    //chomping the initial data we send
    if(recv(csfd,buf,sizeof(buf),0)<=0)
    {
        printf_stderr("protocol_process: recv\n");
        return -1;
    }

    //fulfill the client's wish
    switch(info.cmd)
    {
    case PROTOCOL_INFO_CMD_LIST:
        //send PWD contents of reg files
        if(protocol_server_send_file_list(csfd,path)==-1)
        {
            perror("protocol_process: protocol_server_send_file_list");
            return -1;
        }
        break;
    case PROTOCOL_INFO_CMD_GET:
        //send file from the PWD
        //already been validated that it's not a relitive path
        if(protocol_server_send_file(csfd,path,&info)==-1)
        {
            perror("protocol_process: protocol_server_send_file_list");
            return -1;
        }
        break;
    default:
        protocol_info_cmd_to_string(info.cmd,buf,sizeof(buf));
        printf_stderr("protocol_process: unknown client command \'%s\'\n",buf);
        return -1;
        break;
    }

    return 0;
}
