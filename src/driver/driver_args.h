#ifndef WEV_DRIVER_ARGS_H
#define WEV_DRIVER_ARGS_H

#include "../core/string.h"
#include <stdbool.h>

// The action the driver is asked to execute.
typedef enum DriverAction
{
    DRIVER_ACTION_BUILD,
    DRIVER_ACTION_TEST,
    DRIVER_ACTION_RUN,
    DRIVER_ACTION_VERSION,
    DRIVER_ACTION_HELP,
} DriverAction;

typedef struct DriverArgs
{
    bool success;
    String error_message;

    // Action to execute.
    DriverAction action;

    // True when no action was supplied on the command line.
    bool no_action;
} DriverArgs;

// Parse command ligne arguments and returns the result.
DriverArgs parse_driver_args(int argc, char* argv[]);

#endif // WEV_DRIVER_ARGS_H
