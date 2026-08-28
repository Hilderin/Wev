#include "console.h"

#include <stdio.h>
#include <stdarg.h>

void console_log(Console* console, const char* format, ...)
{
    if (format == NULL) return;

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}
