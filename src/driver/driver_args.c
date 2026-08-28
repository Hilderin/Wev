#include "driver_args.h"

DriverArgs parse_driver_args(int argc, char* argv[])
{
    DriverArgs args = (DriverArgs){.success = false};


    if (argc == 0)
    {
        args.error_message = string_new("No argument specified.");
    }

    if (args.error_message.length > 0)
    {
        args.success = false;
    }

    return args;
}
