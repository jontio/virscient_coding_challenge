#include "server.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>

#include "../common/errormsg.h"

//define for more error information for unit
//#define UNIT_DEBUG_SERVER

#ifndef UNIT_DEBUG_SERVER
#define printf_stderr(x,...) printf_stderr_dummy(x,##__VA_ARGS__)
#define perror(x) perror_dummy(x)
#endif

//--------private

static int n_clients=0;

//listens to a socket
//name: server name (eg 127.0.0.1 or localhost)
//service: port (eg 80 or http)
//return: -1 for error else returns socket file descriptor
int server_listen(const char *name, const char *service)
{
    int sfd;
    int status;
    struct addrinfo hints;
    struct addrinfo *results, *rp;
    int yes=1;

    memset(&hints,0,sizeof(hints));
    hints.ai_family=AF_UNSPEC;     // don't care IPv4 or IPv6
    hints.ai_socktype=SOCK_STREAM; // TCP stream sockets
    hints.ai_flags=AI_PASSIVE;     // For wildcard IP address

    status=getaddrinfo(name,service,&hints,&results);
    if(status!=0)
    {
        printf_stderr("server_listen: getaddrinfo: %s\n",gai_strerror(status));
        return -1;
    }

    //run through results and try to bind
    for(rp=results;rp!=NULL;rp=rp->ai_next)
    {
        sfd=socket(rp->ai_family,rp->ai_socktype,rp->ai_protocol);
        if(sfd==-1)
        {
            continue;
        }
        if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int))==-1)
        {
            perror("server_listen: setsockopt");
            close(sfd);
            return -1;
        }
        if(bind(sfd,rp->ai_addr,rp->ai_addrlen)==0)
        {
            sio_print_address("bound on",rp);
            break;
        }
        close(sfd);
    }
    freeaddrinfo(results);
    if(rp==NULL)
    {
        perror("server_listen: rp==NULL");
        return -1;
    }

    if(listen(sfd,SERVER_LISTEN_BACKLOG)==-1)
    {
        perror("server_listen: listen");
        return -1;
    }

    return sfd;
}

//handles children when they exit
static void server_child_signal_handler(int sig)
{
    (void)sig;
    pid_t pid;
    int status;

    do
    {
        pid=waitpid(-1,&status,WNOHANG);
        if(pid>0)
        {
            //printf("child pid=%d in server_child_signal_handler\n",pid);
            n_clients--;
            //printf("number of clients %d\n",n_clients);
        }
    }while(pid>0);

}

//--------public

int server_start(const char *name, const char *service, const char *dir, int max_nclients, const protocol_process_ptr protocol_process)
{
    int sfd;
    struct sockaddr_storage remote_addr;
    socklen_t sin_size;
    int csfd;
    char s[INET6_ADDRSTRLEN];
    in_port_t remote_port;
    pid_t cpid;
    struct sigaction sa;

    //listen to port
    sfd=server_listen(name, service);
    if(sfd<0)
    {
        return sfd;
    }

    //establish child handler
    //stop zombies and dec n_clients
    sigemptyset(&sa.sa_mask);
    sa.sa_flags=0;
    sa.sa_handler=server_child_signal_handler;
    sigaction(SIGCHLD,&sa,NULL);

    //create child processes to handle each request
    while(1)
    {

        sin_size=sizeof(remote_addr);
        csfd = accept(sfd,(struct sockaddr *)&remote_addr,&sin_size);
        if(csfd==-1)
        {
            perror("server_start: accept");
            //check if sfd has been closed
            if(errno==EBADF)
            {
                return -1;
            }
            continue;
        }

        if(n_clients>=max_nclients)
        {
            close(csfd);
            printf("dropping incomming connection as max number of clients has been reached\n");
            continue;
        }

        if(inet_ntop(remote_addr.ss_family,sio_get_in_addr((struct sockaddr *)&remote_addr),s,sizeof(s))==NULL)
        {
            perror("server_start: inet_ntop");
            close(csfd);
            continue;
        }
        remote_port=sio_get_in_port((struct sockaddr *)&remote_addr);
        printf("server_start: got connection from %s:%u\n",s,ntohs(remote_port));

        cpid=fork();
        if(cpid==-1)
        {
            perror("server_start: fork");
        }
        if(cpid==0)
        {
            //child
            //printf("child pid=%d\n",getpid());
            close(sfd);
            //have someone else deal with the communication protocol
            if(protocol_process(csfd,dir)==-1)
            {
                printf("failed to fulfill client's request\n");
            }
            close(csfd);
            exit(EXIT_SUCCESS);
        }
        //parent
        close(csfd);
        n_clients++;
        //printf("number of clients %d\n",n_clients);

    }

}
