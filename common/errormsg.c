#include "errormsg.h"
#include <stdio.h>
#include <stdarg.h>

void printf_stderr_dummy(const char *fmt, ...)
{
    (void)fmt;
}

void perror_dummy(const char *x)
{
    (void)x;
}

int printf_stderr(const char *fmt, ...)
{
    va_list arg;
    int done;
    va_start(arg,fmt);
    done=vfprintf(stderr,fmt,arg);
    va_end(arg);
    return done;
}
