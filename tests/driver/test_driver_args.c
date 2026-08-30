#include "driver/driver_args.h"
#include "test_util.h"

TEST(test_parse_driver_args_build)
{
    char* argv[] = {"wev", "build"};
    DriverArgs args = parse_driver_args(2, argv);

    ASSERT_TRUE(args.success);
    ASSERT_EQ(args.action, DRIVER_ACTION_BUILD);
    ASSERT_EQ(args.error_message.length, 0);
}

TEST(test_parse_driver_args_test)
{
    char* argv[] = {"wev", "test"};
    DriverArgs args = parse_driver_args(2, argv);

    ASSERT_TRUE(args.success);
    ASSERT_EQ(args.action, DRIVER_ACTION_TEST);
}

TEST(test_parse_driver_args_run)
{
    char* argv[] = {"wev", "run"};
    DriverArgs args = parse_driver_args(2, argv);

    ASSERT_TRUE(args.success);
    ASSERT_EQ(args.action, DRIVER_ACTION_RUN);
}

TEST(test_parse_driver_args_version)
{
    char* argv[] = {"wev", "version"};
    DriverArgs args = parse_driver_args(2, argv);

    ASSERT_TRUE(args.success);
    ASSERT_EQ(args.action, DRIVER_ACTION_VERSION);
}

TEST(test_parse_driver_args_help)
{
    char* argv[] = {"wev", "help"};
    DriverArgs args = parse_driver_args(2, argv);

    ASSERT_TRUE(args.success);
    ASSERT_EQ(args.action, DRIVER_ACTION_HELP);
}

TEST(test_parse_driver_args_no_argument)
{
    char* argv[] = {"wev"};
    DriverArgs args = parse_driver_args(1, argv);

    ASSERT_TRUE(!args.success);
    ASSERT_TRUE(args.no_action);
    ASSERT_EQ(args.error_message.length, 0);
}

TEST(test_parse_driver_args_unknown_action)
{
    char* argv[] = {"wev", "unknown"};
    DriverArgs args = parse_driver_args(2, argv);

    ASSERT_TRUE(!args.success);
    ASSERT_TRUE(args.error_message.length > 0);
}

TEST(test_parse_driver_args_null_action)
{
    char* argv[] = {"wev", NULL};
    DriverArgs args = parse_driver_args(2, argv);

    ASSERT_TRUE(!args.success);
    ASSERT_TRUE(args.error_message.length > 0);
}

RUN_ALL_TESTS()
