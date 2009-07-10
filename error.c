#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "error.h"

/* A wrapper for fprintf that will crash the program if errorlevel is ERR_FATAL
 * */
void
error (enum errorlevel errorlevel, char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    if (errorlevel == ERR_FATAL)
        exit(EXIT_FAILURE);
}
