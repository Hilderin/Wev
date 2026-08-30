#include "driver_args.h"

#include <string.h>

// Parse the action from the first command line argument.
static bool parse_action(const char* arg, DriverAction* out_action)
{
    if (arg == NULL)
    {
        return false;
    }

    if (strcmp(arg, "build") == 0)
    {
        *out_action = DRIVER_ACTION_BUILD;
        return true;
    }

    if (strcmp(arg, "test") == 0)
    {
        *out_action = DRIVER_ACTION_TEST;
        return true;
    }

    if (strcmp(arg, "run") == 0)
    {
        *out_action = DRIVER_ACTION_RUN;
        return true;
    }

    if (strcmp(arg, "version") == 0)
    {
        *out_action = DRIVER_ACTION_VERSION;
        return true;
    }

    if (strcmp(arg, "help") == 0)
    {
        *out_action = DRIVER_ACTION_HELP;
        return true;
    }

    return false;
}

DriverArgs parse_driver_args(int argc, char* argv[])
{
    DriverArgs args = (DriverArgs){.success = false};

    // argv[0] is the program name; the action is the first real argument.
    if (argc < 2)
    {
        args.no_action = true;
        return args;
    }

    const char* action_arg = argv[1];
    if (action_arg == NULL)
    {
        args.error_message = string_new("No action specified.");
        return args;
    }

    if (!parse_action(action_arg, &args.action))
    {
        const char* unknown = "Unknown action. Expected build, test, run, version or help.";
        args.error_message = string_new(unknown);
        return args;
    }

    args.success = true;
    return args;
}
