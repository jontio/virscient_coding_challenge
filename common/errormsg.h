#ifndef ERROR_MSGS_H
#define ERROR_MSGS_H

//these functions don't do anything
void printf_stderr_dummy(const char *fmt, ...);
void perror_dummy(const char *x);

//printf to stderror
int printf_stderr(const char *fmt, ...);

#endif

