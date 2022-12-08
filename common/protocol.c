#include "protocol.h"
#include "sio.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "../common/errormsg.h"

//define for more error information for unit
//#define UNIT_DEBUG_PROTOCOL

#ifndef UNIT_DEBUG_PROTOCOL
#define printf_stderr(x,...) printf_stderr_dummy(x,##__VA_ARGS__)
#define perror(x) perror_dummy(x)
#endif

int protocol_info_cmd_to_string(int cmd,char *buf,int buf_size)
{
    switch(cmd)
    {
    case PROTOCOL_INFO_CMD_LIST:
        if(snprintf(buf,buf_size,"LIST")>=buf_size)return -1;
        break;
    case PROTOCOL_INFO_CMD_GET:
        if(snprintf(buf,buf_size,"GET")>=buf_size)return -1;
        break;
    case PROTOCOL_INFO_CMD_UNSET:
        if(snprintf(buf,buf_size,"UNSET")>=buf_size)return -1;
        break;
    default:
        if(snprintf(buf,buf_size,"UNKNOWN")>=buf_size)return -1;
        break;
    }
    return 0;
}

int protocol_info_string_cmd(const char *buf)
{
    if(strcmp(buf,"LIST")==0)
    {
        return PROTOCOL_INFO_CMD_LIST;
    }
    else if(strcmp(buf,"GET")==0)
    {
        return PROTOCOL_INFO_CMD_GET;
    }
    else if(strcmp(buf,"UNSET")==0)
    {
        return PROTOCOL_INFO_CMD_UNSET;
    }
    return -1;
}

int protocol_validate_info(const char *buf, int buf_size)
{
    int valid;
    int k;
    int len;

    //check str is null terminated and chars are printable or \n
    valid=0;
    for(k=0;k<buf_size;k++)
    {
        if(buf[k]==0)
        {
            valid=1;
            break;
        }
        else if(!isprint(buf[k])&&buf[k]!='\n')
        {
            break;
        }
    }
    if(valid==0)
    {
        return valid;
    }

    //check for ..
    len=strlen(buf);
    valid=1;
    for(k=0;(k+1)<len;k++)
    {
        if(buf[k]!=buf[k+1])continue;
        if(buf[k]!='.')continue;
        valid=0;
        break;
    }

    return valid;
}

int protocol_populate_info(char *buf, int buf_size, protocol_info_t *info)
{
    const char s[2] = ":";
    char *arg,*value;
    int len,k;
    char cmd_string[10];

    //check if string is ok
    if(!protocol_validate_info(buf,buf_size))
    {
        return 0;
    }

    //remove trainling \n if it exists
    len=strlen(buf);
    if(buf[len-1]=='\n')
    {
        buf[len-1]=0;
    }

    //get argument
    arg=strtok(buf,s);
    if(arg==NULL)
    {

        info->finalized=1;

        protocol_info_cmd_to_string(info->cmd,cmd_string,sizeof(cmd_string));

//        printf("PROTO: %d\n",info->proto);
//        printf("VERSION: %d\n",info->version);
//        printf("FILENAME: \'%s\'\n",info->filename);
//        printf("CMD: %s\n",cmd_string);
//        printf("MSG: %s\n",info->msg);
//        printf("OK: %d\n",info->ok);

        return 1;
    }

    //get value
    value=strtok(NULL, s);
    if(value==NULL)
    {
        fprintf(stderr,"null value\n");
        return 0;
    }

    //skip any inital spaces of the value
    len=strlen(value);
    for(k=0;k<len&&value[k]==' ';value++);

    if(len>PROTOCOL_BUF_SIZE)
    {
        fprintf(stderr,"value to big\n");
        return 0;
    }

    //set the argument in info to the user value
    if(strcmp(arg,"FILENAME")==0)
    {
        strcpy(info->filename,value);
    }
    else if(strcmp(arg,"VERSION")==0)
    {
        info->version=strtod(value,NULL);
    }
    else if(strcmp(arg,"PROTO")==0)
    {
        info->proto=strtod(value,NULL);
    }
    else if(strcmp(arg,"CMD")==0)
    {
        info->cmd=protocol_info_string_cmd(value);
    }
    else if(strcmp(arg,"MSG")==0)
    {
        strcpy(info->msg,value);
    }
    else if(strcmp(arg,"OK")==0)
    {
        info->ok=strtod(value,NULL);
    }
    else
    {
        fprintf(stderr,"Unknown argument \'%s\'\n",arg);
        return 0;
    }

    return 1;
}

int protocol_create_message(char *buf,int buf_size,int ok,const char *fmt, ...)
{
    char buf2[PROTOCOL_BUF_SIZE];
    va_list args;
    va_start(args,fmt);
    if(vsnprintf(buf2,sizeof(buf2),fmt,args)>=(int)sizeof(buf2))
    {
        va_end(args);
        return -1;
    }
    va_end(args);

    if(snprintf(buf,buf_size,
             "PROTO: 1\n"
             "VERSION: 1\n"
             "MSG: %s\n"
             "OK: %d\n"
             "\n",buf2,ok
             )>=buf_size)
    {
        return -1;
    }

    return 0;
}

int protocol_get_response_request(sio_cbuf_t *cbuf,protocol_info_t *info)
{
    char buf[PROTOCOL_BUF_SIZE];
    int res;

    //fill in info given to us by the client
    //trailing \n is changed to 0
    do
    {
        res=sio_getline(cbuf,buf,sizeof(buf));
        if(res>0)
        {
            if(protocol_populate_info(buf,sizeof(buf),info)==0)
            {
                printf_stderr("protocol_get_response_request: invalid data from remote\n");
                return -1;
            }
        }
    }while(res>1);
    if(res<=0||!info->finalized)
    {
        printf_stderr("protocol_get_response_request: client gone\n");
        return -1;
    }

    return 0;
}

