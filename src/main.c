#include "driver/driver.h"
#include "driver/console.h"

// Entrypoint of the driver
int main(const int argc, char* argv[])
{
    Console console;
    const Driver driver = (Driver){.console = &console};

    return driver_run(&driver, argc, argv);
}
