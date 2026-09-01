#include "driver/console.h"
#include "driver/driver.h"

// Entrypoint of the driver
int main(const int argc, char* argv[])
{
    Console console = {0};
    const Driver driver = (Driver){.console = &console};

    return driver_run(&driver, argc, argv);
}
