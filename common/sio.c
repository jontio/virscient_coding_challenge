#include "sio.h"

#include <stdio.h>
#include <string.h>

int sio_print_address(const char *text, struct addrinfo *addr)
{

    char ipstr[INET6_ADDRSTRLEN];

    if(addr==NULL)
    {
        fprintf(stderr,"sio_print_address: NULL pointer\n");
        return -1;
    }

    if(addr->ai_addr==NULL)
    {
        fprintf(stderr,"sio_print_address: invalid address\n");
        return -1;
    }

    // convert the IP to a string and print it:
    if(inet_ntop(addr->ai_family,sio_get_in_addr(addr->ai_addr),ipstr,sizeof(ipstr))==NULL)
    {
        perror("sio_print_address: inet_ntop failure");
        return -1;
    }

    if(text==NULL)
    {
        printf("remote address is %s:%u\n",ipstr,ntohs(sio_get_in_port(addr->ai_addr)));
    }
    else
    {
        printf("%s %s:%u\n",text,ipstr,ntohs(sio_get_in_port(addr->ai_addr)));
    }

    return 0;
}

void *sio_get_in_addr(struct sockaddr *sa)
{
    if(sa==NULL)
    {
        return NULL;
    }
    if(sa->sa_family==AF_INET)
    {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

in_port_t sio_get_in_port(struct sockaddr *sa)
{
    if(sa==NULL)
    {
        return 0;
    }
    if(sa->sa_family==AF_INET)
    {
        return (((struct sockaddr_in*)sa)->sin_port);
    }
    return (((struct sockaddr_in6*)sa)->sin6_port);
}

int sio_sendall(int sfd, const char *buf, int len)
{
    int total=0;        // how many bytes we've sent
    int bytesleft=len; // how many we have left to send
    int n;
    while(total<len)
    {
        n=send(sfd,buf+total,bytesleft,0);
        if(n==-1)
        {
            break;
        }
        total+=n;
        bytesleft-=n;
    }
    return n==-1?-1:total; // return -1 on failure, or number actually sent
}

int sio_sendallstr(int sfd, const char *buf)
{
    return sio_sendall(sfd,buf,strlen(buf));
}

int sio_getline(sio_cbuf_t *cbuf,char *buf,int buf_size)
{
    int k=0;
    int nread;
    while(1)
    {

        while(cbuf->rpos!=cbuf->wpos)
        {

            /*
            //if you wanted to drop on line too large
            //then something like this would work
            if(k==buf_size)
            {
                fprintf(stderr, "line too large, dropping\n");
            }
            if(k>=buf_size)
            {
                cbuf->rpos++;
                if(cbuf->buf[cbuf->rpos-1]=='\n')
                {
                    k=0;
                }
                continue;
            }
            */

            if(k>=buf_size)
            {
                fprintf(stderr, "line too large, failing\n");
                return -1;
            }

            buf[k++]=cbuf->buf[cbuf->rpos++];
            if(buf[k-1]=='\n')
            {
                buf[k]=0;
                return k;
            }

        }

        cbuf->wpos=0;
        cbuf->rpos=0;
        nread=recv(cbuf->sfd,cbuf->buf,SIO_CBUF_BUF_SIZE,0);
        if(nread<=0)
        {
            return nread;
        }
        cbuf->wpos=nread;

    }
}

