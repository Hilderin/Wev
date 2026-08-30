#include <stdlib.h>
#include <string.h>

#include "driver/driver.h"
#include "test_util.h"

static Console make_console(void)
{
    Console console = (Console){
        .keep_output = true,
        .std_out = string_builder_new(),
    };
    return console;
}

TEST(test_driver_run_help)
{
    Console console = make_console();
    const Driver driver = (Driver){.console = &console};

    char* argv[] = {"wev", "help"};
    const int result = driver_run(&driver, 2, argv);

    ASSERT_EQ(result, 0);
    ASSERT_TRUE(strstr(console.std_out.buffer, "Usage: wev <action>") != NULL);
    ASSERT_TRUE(strstr(console.std_out.buffer, "help     Print this help") != NULL);

    free(console.std_out.buffer);
}

TEST(test_driver_run_help_documents_all_actions)
{
    Console console = make_console();
    const Driver driver = (Driver){.console = &console};

    char* argv[] = {"wev", "help"};
    const int result = driver_run(&driver, 2, argv);

    ASSERT_EQ(result, 0);
    ASSERT_TRUE(strstr(console.std_out.buffer, "build") != NULL);
    ASSERT_TRUE(strstr(console.std_out.buffer, "test") != NULL);
    ASSERT_TRUE(strstr(console.std_out.buffer, "run") != NULL);
    ASSERT_TRUE(strstr(console.std_out.buffer, "version") != NULL);
    ASSERT_TRUE(strstr(console.std_out.buffer, "help") != NULL);

    free(console.std_out.buffer);
}

TEST(test_driver_run_no_action_prints_help)
{
    Console console = make_console();
    const Driver driver = (Driver){.console = &console};

    char* argv[] = {"wev"};
    const int result = driver_run(&driver, 1, argv);

    ASSERT_EQ(result, 0);
    ASSERT_TRUE(strstr(console.std_out.buffer, "Usage: wev <action>") != NULL);
    ASSERT_TRUE(strstr(console.std_out.buffer, "Actions:") != NULL);

    free(console.std_out.buffer);
}

TEST(test_driver_run_version)
{
    Console console = make_console();
    const Driver driver = (Driver){.console = &console};

    char* argv[] = {"wev", "version"};
    const int result = driver_run(&driver, 2, argv);

    ASSERT_EQ(result, 0);
    ASSERT_TRUE(strstr(console.std_out.buffer, "wev ") != NULL);

    free(console.std_out.buffer);
}

TEST(test_driver_run_unknown_action_returns_error)
{
    Console console = make_console();
    const Driver driver = (Driver){.console = &console};

    char* argv[] = {"wev", "unknown"};
    const int result = driver_run(&driver, 2, argv);

    ASSERT_EQ(result, 1);
    ASSERT_TRUE(strstr(console.std_out.buffer, "Unknown action") != NULL);
    ASSERT_TRUE(strstr(console.std_out.buffer, "Run 'wev help' for usage.") != NULL);

    free(console.std_out.buffer);
}

RUN_ALL_TESTS()
