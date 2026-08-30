#include "driver.h"

#include "driver_args.h"
#include "version.h"

static void print_help(const Driver* driver)
{
    console_log(driver->console, "Usage: wev <action>");
    console_log(driver->console, "");
    console_log(driver->console, "Actions:");
    console_log(driver->console, "  build    Compile a .wev source file");
    console_log(driver->console, "  test     Run the tests");
    console_log(driver->console, "  run      Compile and run a .wev source file");
    console_log(driver->console, "  version  Print the version");
    console_log(driver->console, "  help     Print this help");
}

int driver_run(const Driver* driver, const int argc, char* argv[])
{
    const DriverArgs args = parse_driver_args(argc, argv);

    if (!args.success)
    {
        if (args.no_action)
        {
            print_help(driver);
            return 0;
        }

        console_log(driver->console, args.error_message.content);
        console_log(driver->console, "Run 'wev help' for usage.");
        return 1;
    }

    if (args.action == DRIVER_ACTION_HELP)
    {
        print_help(driver);
        return 0;
    }

    if (args.action == DRIVER_ACTION_VERSION)
    {
        console_log(driver->console, "wev %s", WEV_VERSION);
        return 0;
    }

    return 0;
}
