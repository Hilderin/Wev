#ifndef WEV_DRIVER_H
#define WEV_DRIVER_H

#include "console.h"

typedef struct Driver
{
    Console* console;
} Driver;

int driver_run(const Driver* driver, int argc, char* argv[]);

#endif // WEV_DRIVER_H
