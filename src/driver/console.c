#include "console.h"

#include <stdarg.h>
#include <stdio.h>

void console_log(Console* console, const char* format, ...)
{
    if (console == NULL || format == NULL) return;

    va_list args;
    va_start(args, format);

    if (console->keep_output)
    {
        string_builder_append_vfmt(&console->std_out, format, args);
        string_builder_append(&console->std_out, "\n");
    }

    vprintf(format, args);
    va_end(args);
    printf("\n");
}