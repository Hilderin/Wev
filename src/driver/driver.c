#include "driver.h"

#include "driver_args.h"


static void print_header(const Driver* driver)
{
    console_log(driver->console, "This is Wev Compiler");
}

int driver_run(const Driver* driver, const int argc, char* argv[])
{
    print_header(driver);

    const DriverArgs args = parse_driver_args(argc, argv);

    if (!args.success)
    {
        console_log(driver->console, args.error_message.content);
        return 1;
    }

    return 0;
}
