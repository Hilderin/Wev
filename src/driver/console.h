#ifndef WEV_CONSOLE_H
#define WEV_CONSOLE_H

typedef struct Console
{

} Console;

// Log to the console.
void console_log(Console *console, const char *__restrict format, ...);

#endif //WEV_CONSOLE_H
