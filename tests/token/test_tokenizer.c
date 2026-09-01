#include <string.h>

#include "test_util.h"
#include "token/tokenizer.h"

TEST(test_tokenizer_new)
{
    const Tokenizer tokenizer = tokenizer_new("");

    ASSERT_STR_EQ(tokenizer.content, "");
}

TEST(test_tokenizer_get_next_token_empty)
{
    Tokenizer tokenizer = tokenizer_new("");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_EOF);
    ASSERT_EQ(token.col_number, 1);
    ASSERT_EQ(token.length, 0);
    ASSERT_EQ(token.line_number, 1);
    ASSERT_EQ(token.location, 0);
}

TEST(test_tokenizer_get_next_token_space)
{
    Tokenizer tokenizer = tokenizer_new(" ");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_EOF);
    ASSERT_EQ(token.col_number, 2);
    ASSERT_EQ(token.length, 0);
    ASSERT_EQ(token.line_number, 1);
    ASSERT_EQ(token.location, 1);
}

TEST(test_tokenizer_get_next_token_lf)
{
    Tokenizer tokenizer = tokenizer_new("\n");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_EOF);
    ASSERT_EQ(token.col_number, 1);
    ASSERT_EQ(token.length, 0);
    ASSERT_EQ(token.line_number, 2);
    ASSERT_EQ(token.location, 1);
}

TEST(test_tokenizer_get_next_token_fn)
{
    Tokenizer tokenizer = tokenizer_new("fn");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_FUNCTION);
    ASSERT_EQ(token.col_number, 1);
    ASSERT_EQ(token.length, 2);
    ASSERT_EQ(token.line_number, 1);
    ASSERT_EQ(token.location, 0);
}

RUN_ALL_TESTS()
