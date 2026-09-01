#include <string.h>

#include "test_util.h"
#include "token/token_info.h"

TEST(test_get_token_info_found)
{
    const TokenInfo* info = get_token_info("fn", 2);

    ASSERT_TRUE(info != NULL);
    ASSERT_EQ(info->type, TOKEN_FUNCTION);
    ASSERT_STR_EQ(info->name, "function");
    ASSERT_STR_EQ(info->str, "fn");
    ASSERT_EQ(info->len, 2);
}

TEST(test_get_token_info_not_found)
{
    ASSERT_TRUE(get_token_info("foo", 3) == NULL);
    ASSERT_TRUE(get_token_info("", 0) == NULL);
    ASSERT_TRUE(get_token_info("unknown", 7) == NULL);
    ASSERT_TRUE(get_token_info("eof", 3) == NULL);
}

TEST(test_get_token_info_respects_len)
{
    ASSERT_TRUE(get_token_info("fn", 1) == NULL);
    ASSERT_TRUE(get_token_info("fnx", 3) == NULL);
}

RUN_ALL_TESTS()
