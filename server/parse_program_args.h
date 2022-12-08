#ifndef PARSE_PROGRAM_ARGS_H
#define PARSE_PROGRAM_ARGS_H

#ifndef PARSE_PROGRAM_ARGS_DEFAULT_SERVER_NAME
#define PARSE_PROGRAM_ARGS_DEFAULT_SERVER_NAME NULL
#endif
#ifndef PARSE_PROGRAM_ARGS_DEFAULT_SERVER_PORT
#define PARSE_PROGRAM_ARGS_DEFAULT_SERVER_PORT "12345"
#endif
#ifndef PARSE_PROGRAM_ARGS_DEFAULT_DIRECTORY
#define PARSE_PROGRAM_ARGS_DEFAULT_DIRECTORY "."
#endif
#ifndef PARSE_PROGRAM_ARGS_DEFAULT_MAX_NCLIENTS
#define PARSE_PROGRAM_ARGS_DEFAULT_MAX_NCLIENTS 10
#endif

typedef struct parse_program_args_info
{
    char *dir;
    char *server_port;
    char *server_name;
    int max_nclients;
} parse_program_args_info_t;

//parse program arguments
//argc and argv: from main
//info: is filled in by parse_program_args
//return -1 if error 0 otherwise
int parse_program_args(int argc, char *argv[], parse_program_args_info_t *info);


#endif
