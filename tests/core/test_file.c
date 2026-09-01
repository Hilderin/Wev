#include <stdlib.h>
#include <string.h>

#include "core/file.h"
#include "test_util.h"

#define TEST_TMP_FILE "wev_test_tmp_file.txt"

TEST(test_file_len_null_path)
{
    ASSERT_EQ(file_len(NULL), 0);
}

TEST(test_file_len_missing_file)
{
    ASSERT_EQ(file_len("/this/path/does/not/exist.txt"), 0);
}

TEST(test_file_len_existing_file)
{
    const char* content = "Hello from Wev\n";

    FILE* f = fopen(TEST_TMP_FILE, "w");
    ASSERT_TRUE(f != NULL);
    if (f == NULL) return;

    fwrite(content, 1, strlen(content), f);
    fclose(f);

    ASSERT_EQ(file_len(TEST_TMP_FILE), strlen(content));

    remove(TEST_TMP_FILE);
}

TEST(test_file_len_empty_file)
{
    FILE* f = fopen(TEST_TMP_FILE, "w");
    ASSERT_TRUE(f != NULL);
    if (f == NULL) return;

    fclose(f);

    ASSERT_EQ(file_len(TEST_TMP_FILE), 0);

    remove(TEST_TMP_FILE);
}

TEST(test_file_read_str_null_path)
{
    ASSERT_TRUE(file_read_str(NULL) == NULL);
}

TEST(test_file_read_str_missing_file)
{
    ASSERT_TRUE(file_read_str("/this/path/does/not/exist.txt") == NULL);
}

TEST(test_file_read_str_existing_file)
{
    const char* content = "Hello from Wev\n";

    FILE* f = fopen(TEST_TMP_FILE, "w");
    ASSERT_TRUE(f != NULL);
    if (f == NULL) return;

    fwrite(content, 1, strlen(content), f);
    fclose(f);

    char* str = file_read_str(TEST_TMP_FILE);
    ASSERT_TRUE(str != NULL);
    ASSERT_STR_EQ(str, content);

    free(str);
    remove(TEST_TMP_FILE);
}

TEST(test_file_read_str_empty_file)
{
    FILE* f = fopen(TEST_TMP_FILE, "w");
    ASSERT_TRUE(f != NULL);
    if (f == NULL) return;

    fclose(f);

    char* str = file_read_str(TEST_TMP_FILE);
    ASSERT_TRUE(str != NULL);
    ASSERT_STR_EQ(str, "");

    free(str);
    remove(TEST_TMP_FILE);
}

TEST(test_file_read_str_multiline_file)
{
    const char* content = "line 1\nline 2\nline 3\n";

    FILE* f = fopen(TEST_TMP_FILE, "w");
    ASSERT_TRUE(f != NULL);
    if (f == NULL) return;

    fwrite(content, 1, strlen(content), f);
    fclose(f);

    char* str = file_read_str(TEST_TMP_FILE);
    ASSERT_TRUE(str != NULL);
    ASSERT_STR_EQ(str, content);

    free(str);
    remove(TEST_TMP_FILE);
}

RUN_ALL_TESTS()