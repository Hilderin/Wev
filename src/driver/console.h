#ifndef WEV_CONSOLE_H
#define WEV_CONSOLE_H

#include <stdbool.h>

#include "../core/string_builder.h"

typedef struct Console
{
    bool keep_output; // Used to keep the console prints/log for unit testing.
    StringBuilder std_out;
} Console;

// Log to the console.
void console_log(Console* console, const char* __restrict format, ...);

#endif //WEV_CONSOLE_H
