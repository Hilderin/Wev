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

RUN_ALL_TESTS()