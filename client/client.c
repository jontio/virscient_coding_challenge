#include "client.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../common/errormsg.h"

//define for more error information for unit
//#define UNIT_DEBUG_CLIENT

#ifndef UNIT_DEBUG_CLIENT
#define printf_stderr(x,...) printf_stderr_dummy(x,##__VA_ARGS__)
#define perror(x) perror_dummy(x)
#endif

//-----private

//-----public

int client_disconnect(int sfd)
{
    return close(sfd);
}

int client_connect(const char *name, const char *service, struct addrinfo *addr)
{

    int sfd;
    int status;
    struct addrinfo hints;
    struct addrinfo *results, *rp;

    if(addr!=NULL)
    {
        memset(addr,0,sizeof(struct addrinfo));
    }

    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype=SOCK_STREAM; // TCP stream sockets

    status=getaddrinfo(name,service,&hints,&results);
    if(status!=0)
    {
        printf_stderr("client_start: getaddrinfo error: %s\n",gai_strerror(status));
        return -1;
    }

    //run through results and try to connect
    for(rp=results;rp!=NULL;rp=rp->ai_next)
    {
        sfd=socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol);
        if(sfd==-1)
        {
            continue;
        }
        if(connect(sfd,rp->ai_addr,rp->ai_addrlen)==0)
        {
            break;
        }
        close(sfd);
    }
    if(rp!=NULL)
    {
        if(addr!=NULL)
        {
            memcpy(addr,rp,sizeof(struct addrinfo));
        }
    }
    freeaddrinfo(results);
    if(rp==NULL)
    {
        printf_stderr("client_connect: Could not connect\n");
        return -1;
    }

    return sfd;
}
