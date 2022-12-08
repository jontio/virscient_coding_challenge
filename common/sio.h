#ifndef SIO_H
#define SIO_H

#include <arpa/inet.h>
#include <netdb.h>

//max length of buffer that is used by sio_getline
#ifndef SIO_CBUF_BUF_SIZE
#define SIO_CBUF_BUF_SIZE 16
#endif

//struct for sio_getline
typedef struct sio_cbuf
{
    char buf[SIO_CBUF_BUF_SIZE];
    int sfd;
    int rpos, wpos;
} sio_cbuf_t;

//prints to stdout address and port given by addr
//text is prepended to output
//return: -1 for error else 0
int sio_print_address(const char *text, struct addrinfo *addr);

//returns a void pointer to either sockaddr_in6 or sockaddr_in
//depending on sa->sa_family
void *sio_get_in_addr(struct sockaddr *sa);

//returns port of sa
in_port_t sio_get_in_port(struct sockaddr *sa);

//tries to send all of buffer
//if it fails returns -1 else returns number of bytes sent
int sio_sendall(int sfd, const char *buf, int len);

//same as sio_sendall but is for \0 terminated char buffer
int sio_sendallstr(int sfd, const char *buf);

//tries to get a line into buf ending in \n
//returns -1 for error and 0 for when socket is shutdown
//otherwise it returns the number of bytes returned in buf
int sio_getline(sio_cbuf_t *cbuf,char *buf,int buf_size);

#endif

