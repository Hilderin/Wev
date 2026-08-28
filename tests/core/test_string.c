#include <string.h>

#include "core/file.h"
#include "core/string.h"
#include "test_util.h"

TEST(test_string_empty)
{
    String s = string_empty();

    ASSERT_TRUE(s.content != NULL);
    ASSERT_EQ(s.length, 1);
    ASSERT_EQ(s.capacity, 1);
    ASSERT_EQ(s.content[0], '\0');

    string_free(&s);
}

TEST(test_string_new)
{
    String s = string_new("hello");

    ASSERT_TRUE(s.content != NULL);
    ASSERT_EQ(s.length, 5);
    ASSERT_EQ(s.capacity, 6);
    ASSERT_EQ(s.content[5], '\0');
    ASSERT_STR_EQ(s.content, "hello");

    string_free(&s);
}

TEST(test_string_new_null_returns_empty)
{
    String s = string_new(NULL);

    ASSERT_EQ(s.length, 1);
    ASSERT_EQ(s.capacity, 1);

    string_free(&s);
}

TEST(test_string_new_empty_string)
{
    String s = string_new("");

    ASSERT_EQ(s.length, 1);
    ASSERT_EQ(s.capacity, 1);

    string_free(&s);
}

TEST(test_string_new_len)
{
    String s = string_new_len("hello world", 5);

    ASSERT_TRUE(s.content != NULL);
    ASSERT_EQ(s.length, 5);
    ASSERT_EQ(s.capacity, 6);
    ASSERT_TRUE(strncmp(s.content, "hello", 5) == 0);
    ASSERT_EQ(s.content[5], '\0');

    string_free(&s);
}

TEST(test_string_new_len_null_returns_empty)
{
    String s = string_new_len(NULL, 5);

    ASSERT_EQ(s.length, 1);

    string_free(&s);
}

TEST(test_string_free_null_does_not_crash)
{
    string_free(NULL);
}

RUN_ALL_TESTS()