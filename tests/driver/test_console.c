#include <stdlib.h>
#include <string.h>

#include "driver/console.h"
#include "test_util.h"

static Console make_console(const bool keep_output)
{
    Console console = (Console){
        .keep_output = keep_output,
        .std_out = string_builder_new(),
    };
    return console;
}

TEST(test_console_keep_output_captures_format)
{
    Console console = make_console(true);

    console_log(&console, "Hello %s!", "world");

    ASSERT_STR_EQ(console.std_out.buffer, "Hello world!\n");

    free(console.std_out.buffer);
}

TEST(test_console_keep_output_accumulates)
{
    Console console = make_console(true);

    console_log(&console, "line 1");
    console_log(&console, "line %d", 2);
    console_log(&console, "line 3");

    ASSERT_STR_EQ(console.std_out.buffer, "line 1\nline 2\nline 3\n");

    free(console.std_out.buffer);
}

TEST(test_console_keep_output_false_ignores)
{
    Console console = make_console(false);

    console_log(&console, "should not be kept");

    ASSERT_EQ(console.std_out.length, 0);

    free(console.std_out.buffer);
}

TEST(test_console_null_safe)
{
    Console console = make_console(true);

    console_log(NULL, "nope");
    console_log(&console, NULL);

    ASSERT_EQ(console.std_out.length, 0);

    free(console.std_out.buffer);
}

RUN_ALL_TESTS()