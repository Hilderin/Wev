#include <stdlib.h>
#include <string.h>

#include "core/string_builder.h"
#include "test_util.h"

TEST(test_builder_new_is_empty)
{
    StringBuilder b = string_builder_new();

    ASSERT_EQ(b.length, 0);
    ASSERT_EQ(b.capacity, 0);

    free(b.buffer);
}

TEST(test_builder_append)
{
    StringBuilder b = string_builder_new();

    string_builder_append(&b, "hello");
    ASSERT_EQ(b.length, 5);
    ASSERT_STR_EQ(b.buffer, "hello");

    string_builder_append(&b, " world");
    ASSERT_EQ(b.length, 11);
    ASSERT_STR_EQ(b.buffer, "hello world");
    ASSERT_EQ(b.buffer[b.length], '\0');

    free(b.buffer);
}

TEST(test_builder_append_null_does_nothing)
{
    StringBuilder b = string_builder_new();

    string_builder_append(&b, NULL);
    string_builder_append(NULL, "hello");
    string_builder_append(&b, "");

    ASSERT_EQ(b.length, 0);

    free(b.buffer);
}

TEST(test_builder_append_grows_capacity)
{
    StringBuilder b = string_builder_new();

    for (int i = 0; i < 100; i++)
    {
        string_builder_append(&b, "abc");
    }

    ASSERT_EQ(b.length, 300);
    ASSERT_TRUE(b.capacity >= b.length + 1);
    ASSERT_TRUE(strncmp(b.buffer, "abc", 3) == 0);
    ASSERT_EQ(b.buffer[b.length], '\0');

    free(b.buffer);
}

TEST(test_builder_append_fmt)
{
    StringBuilder b = string_builder_new();

    string_builder_append_fmt(&b, "%d + %d = %d", 1, 2, 3);
    ASSERT_STR_EQ(b.buffer, "1 + 2 = 3");

    string_builder_append_fmt(&b, " and %s", "more");
    ASSERT_STR_EQ(b.buffer, "1 + 2 = 3 and more");
    ASSERT_EQ(b.buffer[b.length], '\0');

    free(b.buffer);
}

TEST(test_builder_free_resets_fields)
{
    StringBuilder b = string_builder_new();
    string_builder_append(&b, "hello");
    ASSERT_EQ(b.length, 5);
    ASSERT_TRUE(b.buffer != NULL);

    string_builder_free(&b);

    ASSERT_TRUE(b.buffer == NULL);
    ASSERT_EQ(b.length, 0);
    ASSERT_EQ(b.capacity, 0);
}

TEST(test_builder_free_null_does_not_crash)
{
    string_builder_free(NULL);
}

TEST(test_builder_free_twice_does_not_crash)
{
    StringBuilder b = string_builder_new();

    string_builder_free(&b);
    string_builder_free(&b);

    ASSERT_TRUE(b.buffer == NULL);
    ASSERT_EQ(b.length, 0);
    ASSERT_EQ(b.capacity, 0);
}

TEST(test_builder_append_fmt_longer_than_capacity)
{
    StringBuilder b = string_builder_new();

    char expected[256] = {0};
    size_t pos = 0;
    for (int i = 0; i < 50; i++)
    {
        int n = snprintf(expected + pos, sizeof(expected) - pos, "%d", i);
        pos += (size_t)n;
    }

    for (int i = 0; i < 50; i++)
    {
        string_builder_append_fmt(&b, "%d", i);
    }

    ASSERT_EQ(b.length, strlen(expected));
    ASSERT_STR_EQ(b.buffer, expected);
    ASSERT_EQ(b.buffer[b.length], '\0');

    free(b.buffer);
}

RUN_ALL_TESTS()