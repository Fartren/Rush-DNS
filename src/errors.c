#include "errors.h"

#include <stdarg.h>
#include <stdlib.h>

void error(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "Error: ");
    vfprintf(stderr, fmt, va);
    va_end(va);
    exit(EXIT_FAILURE);
}

void warn(const char *fmt, ...)
{
    va_list va;
    va_start(va, fmt);
    fprintf(stderr, "Warning: ");
    vfprintf(stderr, fmt, va);
    va_end(va);
}
