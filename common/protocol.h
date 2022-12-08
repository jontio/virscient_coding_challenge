#ifndef PROTOCOL_H
#define PROTOCOL_H

#ifndef PROTOCOL_BUF_SIZE
#define PROTOCOL_BUF_SIZE 1024
#endif

#ifndef PROTOCOL_RX_TIMEOUT
#define PROTOCOL_RX_TIMEOUT 10
#endif

#include <stdio.h>
#include "sio.h"

enum {PROTOCOL_INFO_CMD_UNSET, PROTOCOL_INFO_CMD_LIST, PROTOCOL_INFO_CMD_GET};
typedef struct protocol_info
{
    char filename[PROTOCOL_BUF_SIZE];
    FILE *fd;
    int version;
    int proto;
    int cmd;
    int finalized;
    int ok;
    char msg[PROTOCOL_BUF_SIZE];
} protocol_info_t;

//returns: -1 if an error happened else fills buffer with
//         human readable version of cmd
int protocol_info_cmd_to_string(int cmd,char *buf,int buf_size);

//return: -1 for invalid cmd else returns value of cmd as
//        defined above enum
int protocol_info_string_cmd(const char *buf);

//return: 0 for invalid 1 for valid
int protocol_validate_info(const char *buf, int buf_size);

//modifies buf by removing trailing \n
//return: 1 if info has been updated, 0 if not
int protocol_populate_info(char *buf, int buf_size, protocol_info_t *info);

//creates a message packet that contains human readable
//          text for the remote device.
//returns: -1 if an error else 0
int protocol_create_message(char *buf, int buf_size, int ok, const char *fmt, ...);

//this may chomp into data not for us as it buffers
//so dont go directly from using this to raw read
int protocol_get_response_request(sio_cbuf_t *cbuf,protocol_info_t *info);

#endif
