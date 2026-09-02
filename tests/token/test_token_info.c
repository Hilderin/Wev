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

TEST(test_get_token_info_keywords)
{
    const TokenInfo* info = get_token_info("struct", 6);
    ASSERT_TRUE(info != NULL);
    ASSERT_EQ(info->type, TOKEN_STRUCT);

    ASSERT_TRUE(get_token_info("return", 6) != NULL);
    ASSERT_TRUE(get_token_info("return", 6)->type == TOKEN_RETURN);
    ASSERT_TRUE(get_token_info("impl", 4) != NULL);
    ASSERT_TRUE(get_token_info("impl", 4)->type == TOKEN_IMPL);
    ASSERT_TRUE(get_token_info("consuming", 9) != NULL);
    ASSERT_TRUE(get_token_info("consuming", 9)->type == TOKEN_CONSUMING);
    ASSERT_TRUE(get_token_info("make", 4) != NULL);
    ASSERT_TRUE(get_token_info("make", 4)->type == TOKEN_MAKE);
    ASSERT_TRUE(get_token_info("unsafe", 6) != NULL);
    ASSERT_TRUE(get_token_info("unsafe", 6)->type == TOKEN_UNSAFE);
    ASSERT_TRUE(get_token_info("include", 7) != NULL);
    ASSERT_TRUE(get_token_info("include", 7)->type == TOKEN_INCLUDE);
}

TEST(test_get_token_info_primitive_types)
{
    ASSERT_TRUE(get_token_info("bool", 4) != NULL);
    ASSERT_TRUE(get_token_info("bool", 4)->type == TOKEN_BOOL);
    ASSERT_TRUE(get_token_info("i8", 2) != NULL);
    ASSERT_TRUE(get_token_info("i8", 2)->type == TOKEN_I8);
    ASSERT_TRUE(get_token_info("i32", 3) != NULL);
    ASSERT_TRUE(get_token_info("i32", 3)->type == TOKEN_I32);
    ASSERT_TRUE(get_token_info("u64", 3) != NULL);
    ASSERT_TRUE(get_token_info("u64", 3)->type == TOKEN_U64);
    ASSERT_TRUE(get_token_info("usize", 5) != NULL);
    ASSERT_TRUE(get_token_info("usize", 5)->type == TOKEN_USIZE);
    ASSERT_TRUE(get_token_info("f64", 3) != NULL);
    ASSERT_TRUE(get_token_info("f64", 3)->type == TOKEN_F64);
    ASSERT_TRUE(get_token_info("char", 4) != NULL);
    ASSERT_TRUE(get_token_info("char", 4)->type == TOKEN_CHAR);
    ASSERT_TRUE(get_token_info("void", 4) != NULL);
    ASSERT_TRUE(get_token_info("void", 4)->type == TOKEN_VOID);
}

TEST(test_get_token_info_prefix_not_keyword)
{
    ASSERT_TRUE(get_token_info("implement", 9) == NULL);
    ASSERT_TRUE(get_token_info("structs", 7) == NULL);
    ASSERT_TRUE(get_token_info("inline", 6) == NULL);
    ASSERT_TRUE(get_token_info("i320", 4) == NULL);
    ASSERT_TRUE(get_token_info("String", 6) == NULL);
}

RUN_ALL_TESTS()
