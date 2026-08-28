#ifndef WEV_DRIVER_ARGS_H
#define WEV_DRIVER_ARGS_H

#include <stdbool.h>
#include "../core/string.h"

typedef struct DriverArgs
{
    bool success;
    String error_message;
} DriverArgs;

// Parse command ligne arguments and returns the result.
DriverArgs parse_driver_args(int argc, char* argv[]);

#endif //WEV_DRIVER_ARGS_H
