#include <string.h>

#include "test_util.h"
#include "token/tokenizer.h"

#define MAX_TOKEN_SEQUENCE 256

typedef struct TokenSequence
{
    const char* content;
    Token tokens[MAX_TOKEN_SEQUENCE];
    uint count;
} TokenSequence;

// Tokenizes the whole content, stopping after TOKEN_EOF.
static TokenSequence tokenize_all(const char* content)
{
    TokenSequence sequence = {.content = content};
    Tokenizer tokenizer = tokenizer_new(content);

    while (sequence.count < MAX_TOKEN_SEQUENCE)
    {
        const Token token = get_next_token(&tokenizer);
        sequence.tokens[sequence.count++] = token;
        if (token.type == TOKEN_EOF)
        {
            break;
        }
    }
    return sequence;
}

// Asserts that the token sequence matches the expected token types.
static void assert_token_types(const TokenSequence* sequence, const TokenType* expected, const uint expected_count)
{
    ASSERT_EQ(sequence->count, expected_count);
    for (uint i = 0; i < expected_count && i < sequence->count; i++)
    {
        ASSERT_EQ(sequence->tokens[i].type, expected[i]);
    }
}

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

TEST(test_tokenizer_get_next_token_keyword)
{
    Tokenizer tokenizer = tokenizer_new("struct User");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_STRUCT);
    ASSERT_EQ(token.length, 6);
    ASSERT_EQ(token.location, 0);
}

TEST(test_tokenizer_get_next_token_identifier)
{
    Tokenizer tokenizer = tokenizer_new("username");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(token.length, 8);
    ASSERT_EQ(token.location, 0);
    ASSERT_TRUE(memcmp(tokenizer.content + token.location, "username", token.length) == 0);
}

TEST(test_tokenizer_get_next_token_declare)
{
    Tokenizer tokenizer = tokenizer_new("x := 42");

    const Token first = get_next_token(&tokenizer);
    const Token second = get_next_token(&tokenizer);
    const Token third = get_next_token(&tokenizer);

    ASSERT_EQ(first.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(first.length, 1);
    ASSERT_EQ(second.type, TOKEN_DECLARE);
    ASSERT_EQ(second.length, 2);
    ASSERT_EQ(third.type, TOKEN_INT_LITERAL);
    ASSERT_EQ(third.length, 2);
}

TEST(test_tokenizer_get_next_token_int_literal)
{
    Tokenizer tokenizer = tokenizer_new("12345");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_INT_LITERAL);
    ASSERT_EQ(token.length, 5);
    ASSERT_EQ(token.location, 0);
}

TEST(test_tokenizer_get_next_token_hex_literal)
{
    Tokenizer tokenizer = tokenizer_new("0xFF");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_INT_LITERAL);
    ASSERT_EQ(token.length, 4);
}

TEST(test_tokenizer_get_next_token_float_literal)
{
    Tokenizer tokenizer = tokenizer_new("3.14");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_FLOAT_LITERAL);
    ASSERT_EQ(token.length, 4);
}

TEST(test_tokenizer_get_next_token_float_exponent)
{
    Tokenizer tokenizer = tokenizer_new("1.5e-3");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_FLOAT_LITERAL);
    ASSERT_EQ(token.length, 6);
}

TEST(test_tokenizer_get_next_token_string_literal)
{
    Tokenizer tokenizer = tokenizer_new("println(\"Hello from Wev\")");

    const Token first = get_next_token(&tokenizer);
    const Token second = get_next_token(&tokenizer);
    const Token third = get_next_token(&tokenizer);
    const Token fourth = get_next_token(&tokenizer);

    ASSERT_EQ(first.type, TOKEN_IDENTIFIER);
    ASSERT_EQ(second.type, TOKEN_LPAREN);
    ASSERT_EQ(third.type, TOKEN_STRING_LITERAL);
    ASSERT_EQ(third.length, 16);
    ASSERT_EQ(tokenizer.content[third.location], '"');
    ASSERT_EQ(fourth.type, TOKEN_RPAREN);
}

TEST(test_tokenizer_get_next_token_char_literal)
{
    Tokenizer tokenizer = tokenizer_new("'a'");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_CHAR_LITERAL);
    ASSERT_EQ(token.length, 3);
}

TEST(test_tokenizer_get_next_token_operators)
{
    Tokenizer tokenizer = tokenizer_new("+= -> == != <= >= && || ?");

    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_PLUS_ASSIGN);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_ARROW);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_EQ);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_NE);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_LE);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_GE);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_AND);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_OR);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_QUESTION);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_EOF);
}

TEST(test_tokenizer_get_next_token_single_operators)
{
    Tokenizer tokenizer = tokenizer_new("{ } ( ) [ ] , ; : . + - * / % & | ! ~ < >");

    const TokenType expected[] = {
        TOKEN_LBRACE, TOKEN_RBRACE, TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACKET, TOKEN_RBRACKET, TOKEN_COMMA,   TOKEN_SEMICOLON,
        TOKEN_COLON,  TOKEN_DOT,    TOKEN_PLUS,   TOKEN_MINUS,  TOKEN_STAR,     TOKEN_SLASH,    TOKEN_PERCENT, TOKEN_AMP,
        TOKEN_PIPE,   TOKEN_NOT,    TOKEN_TILDE,  TOKEN_LT,     TOKEN_GT,       TOKEN_EOF,
    };

    for (uint i = 0; i < sizeof(expected) / sizeof(expected[0]); i++)
    {
        const Token token = get_next_token(&tokenizer);
        ASSERT_EQ(token.type, expected[i]);
    }
}

TEST(test_tokenizer_get_next_token_method_chain)
{
    Tokenizer tokenizer = tokenizer_new("users[0].display()");

    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_IDENTIFIER);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_LBRACKET);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_INT_LITERAL);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_RBRACKET);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_DOT);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_IDENTIFIER);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_LPAREN);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_RPAREN);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_EOF);
}

TEST(test_tokenizer_get_next_token_reference_type)
{
    Tokenizer tokenizer = tokenizer_new("fn name() -> &String");

    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_FUNCTION);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_IDENTIFIER);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_LPAREN);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_RPAREN);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_ARROW);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_AMP);
    ASSERT_EQ(get_next_token(&tokenizer).type, TOKEN_IDENTIFIER);
}

TEST(test_tokenizer_get_next_token_line_comment)
{
    Tokenizer tokenizer = tokenizer_new("// comment\nfn");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_FUNCTION);
    ASSERT_EQ(token.line_number, 2);
}

TEST(test_tokenizer_get_next_token_block_comment)
{
    Tokenizer tokenizer = tokenizer_new("/* block\ncomment */ i32");

    const Token token = get_next_token(&tokenizer);

    ASSERT_EQ(token.type, TOKEN_I32);
    ASSERT_EQ(token.line_number, 2);
}

TEST(test_tokenizer_get_next_token_line_tracking)
{
    Tokenizer tokenizer = tokenizer_new("a\n b\n  c");

    const Token first = get_next_token(&tokenizer);
    const Token second = get_next_token(&tokenizer);
    const Token third = get_next_token(&tokenizer);

    ASSERT_EQ(first.line_number, 1);
    ASSERT_EQ(first.col_number, 1);
    ASSERT_EQ(second.line_number, 2);
    ASSERT_EQ(second.col_number, 2);
    ASSERT_EQ(third.line_number, 3);
    ASSERT_EQ(third.col_number, 3);
}

TEST(test_tokenizer_fn_main)
{
    const TokenSequence sequence = tokenize_all("fn main() {\n    println(\"Hello from Wev\")\n}\n");

    const TokenType expected[] = {
        TOKEN_FUNCTION,
        TOKEN_IDENTIFIER,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_LBRACE,
        TOKEN_IDENTIFIER,
        TOKEN_LPAREN,
        TOKEN_STRING_LITERAL,
        TOKEN_RPAREN,
        TOKEN_RBRACE,
        TOKEN_EOF,
    };

    assert_token_types(&sequence, expected, sizeof(expected) / sizeof(expected[0]));

    ASSERT_EQ(sequence.tokens[0].line_number, 1);
    ASSERT_EQ(sequence.tokens[1].col_number, 4);
    ASSERT_EQ(sequence.tokens[7].length, 16);
}

TEST(test_tokenizer_acceptance_example)
{
    const char* source = "include <stdio.h>\n"
                         "\n"
                         "struct User {\n"
                         "    name: String\n"
                         "    age: i32\n"
                         "}\n"
                         "\n"
                         "impl User {\n"
                         "    fn birthday() {\n"
                         "        age += 1\n"
                         "    }\n"
                         "\n"
                         "    fn display() {\n"
                         "        println(name)\n"
                         "    }\n"
                         "\n"
                         "    consuming fn into_name() -> String {\n"
                         "        return name\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "fn main() {\n"
                         "    users := make(List<User>())\n"
                         "\n"
                         "    user := make(User {\n"
                         "        name: \"Alice\"\n"
                         "        age: 30\n"
                         "    })\n"
                         "\n"
                         "    user.birthday()\n"
                         "    users.push(user)\n"
                         "    users[0].display()\n"
                         "}\n";

    const TokenSequence sequence = tokenize_all(source);

    const TokenType expected[] = {
        TOKEN_INCLUDE,     TOKEN_LT,          TOKEN_IDENTIFIER,  TOKEN_DOT,         TOKEN_IDENTIFIER,
        TOKEN_GT,          TOKEN_STRUCT,      TOKEN_IDENTIFIER,  TOKEN_LBRACE,      TOKEN_IDENTIFIER,
        TOKEN_COLON,       TOKEN_IDENTIFIER,  TOKEN_IDENTIFIER,  TOKEN_COLON,       TOKEN_I32,
        TOKEN_RBRACE,      TOKEN_IMPL,        TOKEN_IDENTIFIER,  TOKEN_LBRACE,      TOKEN_FUNCTION,
        TOKEN_IDENTIFIER,  TOKEN_LPAREN,      TOKEN_RPAREN,      TOKEN_LBRACE,      TOKEN_IDENTIFIER,
        TOKEN_PLUS_ASSIGN, TOKEN_INT_LITERAL, TOKEN_RBRACE,      TOKEN_FUNCTION,    TOKEN_IDENTIFIER,
        TOKEN_LPAREN,      TOKEN_RPAREN,      TOKEN_LBRACE,      TOKEN_IDENTIFIER,  TOKEN_LPAREN,
        TOKEN_IDENTIFIER,  TOKEN_RPAREN,      TOKEN_RBRACE,      TOKEN_CONSUMING,   TOKEN_FUNCTION,
        TOKEN_IDENTIFIER,  TOKEN_LPAREN,      TOKEN_RPAREN,      TOKEN_ARROW,       TOKEN_IDENTIFIER,
        TOKEN_LBRACE,      TOKEN_RETURN,      TOKEN_IDENTIFIER,  TOKEN_RBRACE,      TOKEN_RBRACE,
        TOKEN_FUNCTION,    TOKEN_IDENTIFIER,  TOKEN_LPAREN,      TOKEN_RPAREN,      TOKEN_LBRACE,
        TOKEN_IDENTIFIER,  TOKEN_DECLARE,     TOKEN_MAKE,        TOKEN_LPAREN,      TOKEN_IDENTIFIER,
        TOKEN_LT,          TOKEN_IDENTIFIER,  TOKEN_GT,          TOKEN_LPAREN,      TOKEN_RPAREN,
        TOKEN_RPAREN,      TOKEN_IDENTIFIER,  TOKEN_DECLARE,     TOKEN_MAKE,        TOKEN_LPAREN,
        TOKEN_IDENTIFIER,  TOKEN_LBRACE,      TOKEN_IDENTIFIER,  TOKEN_COLON,       TOKEN_STRING_LITERAL,
        TOKEN_IDENTIFIER,  TOKEN_COLON,       TOKEN_INT_LITERAL, TOKEN_RBRACE,      TOKEN_RPAREN,
        TOKEN_IDENTIFIER,  TOKEN_DOT,         TOKEN_IDENTIFIER,  TOKEN_LPAREN,      TOKEN_RPAREN,
        TOKEN_IDENTIFIER,  TOKEN_DOT,         TOKEN_IDENTIFIER,  TOKEN_LPAREN,      TOKEN_IDENTIFIER,
        TOKEN_RPAREN,      TOKEN_IDENTIFIER,  TOKEN_LBRACKET,    TOKEN_INT_LITERAL, TOKEN_RBRACKET,
        TOKEN_DOT,         TOKEN_IDENTIFIER,  TOKEN_LPAREN,      TOKEN_RPAREN,      TOKEN_RBRACE,
        TOKEN_EOF,
    };

    assert_token_types(&sequence, expected, sizeof(expected) / sizeof(expected[0]));

    ASSERT_EQ(sequence.tokens[5].type, TOKEN_GT);
    ASSERT_EQ(sequence.tokens[74].length, 7);
    ASSERT_TRUE(memcmp(sequence.content + sequence.tokens[74].location, "\"Alice\"", 7) == 0);
    ASSERT_EQ(sequence.tokens[93].length, 1);
}

TEST(test_tokenizer_tab_indentation)
{
    const TokenSequence sequence = tokenize_all("fn main() {\n\tx := 42\n\tif x > 0 {\n\t\tx += 1\n\t}\n}\n");

    const TokenType expected[] = {
        TOKEN_FUNCTION,    TOKEN_IDENTIFIER,  TOKEN_LPAREN,     TOKEN_RPAREN, TOKEN_LBRACE,      TOKEN_IDENTIFIER, TOKEN_DECLARE,
        TOKEN_INT_LITERAL, TOKEN_IF,          TOKEN_IDENTIFIER, TOKEN_GT,     TOKEN_INT_LITERAL, TOKEN_LBRACE,     TOKEN_IDENTIFIER,
        TOKEN_PLUS_ASSIGN, TOKEN_INT_LITERAL, TOKEN_RBRACE,     TOKEN_RBRACE, TOKEN_EOF,
    };

    assert_token_types(&sequence, expected, sizeof(expected) / sizeof(expected[0]));

    ASSERT_EQ(sequence.tokens[5].col_number, 2);
    ASSERT_EQ(sequence.tokens[8].col_number, 2);
    ASSERT_EQ(sequence.tokens[13].col_number, 3);
    ASSERT_EQ(sequence.tokens[13].line_number, 4);
}

TEST(test_tokenizer_crlf)
{
    const TokenSequence sequence = tokenize_all("fn main() {\r\n\tcount: usize = 10\r\n\tcount = 20\r\n}\r\n");

    const TokenType expected[] = {
        TOKEN_FUNCTION,
        TOKEN_IDENTIFIER,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_LBRACE,
        TOKEN_IDENTIFIER,
        TOKEN_COLON,
        TOKEN_USIZE,
        TOKEN_ASSIGN,
        TOKEN_INT_LITERAL,
        TOKEN_IDENTIFIER,
        TOKEN_ASSIGN,
        TOKEN_INT_LITERAL,
        TOKEN_RBRACE,
        TOKEN_EOF,
    };

    assert_token_types(&sequence, expected, sizeof(expected) / sizeof(expected[0]));

    ASSERT_EQ(sequence.tokens[5].line_number, 2);
    ASSERT_EQ(sequence.tokens[5].col_number, 2);
    ASSERT_EQ(sequence.tokens[10].line_number, 3);
    ASSERT_EQ(sequence.tokens[13].line_number, 4);
}

TEST(test_tokenizer_cr_only)
{
    const TokenSequence sequence = tokenize_all("fn main() {\rx := 1\ry := 2\r}\r");

    const TokenType expected[] = {
        TOKEN_FUNCTION,
        TOKEN_IDENTIFIER,
        TOKEN_LPAREN,
        TOKEN_RPAREN,
        TOKEN_LBRACE,
        TOKEN_IDENTIFIER,
        TOKEN_DECLARE,
        TOKEN_INT_LITERAL,
        TOKEN_IDENTIFIER,
        TOKEN_DECLARE,
        TOKEN_INT_LITERAL,
        TOKEN_RBRACE,
        TOKEN_EOF,
    };

    assert_token_types(&sequence, expected, sizeof(expected) / sizeof(expected[0]));

    ASSERT_EQ(sequence.tokens[5].line_number, 2);
    ASSERT_EQ(sequence.tokens[5].col_number, 1);
    ASSERT_EQ(sequence.tokens[8].line_number, 3);
    ASSERT_EQ(sequence.tokens[11].line_number, 4);
}

TEST(test_tokenizer_crlf_comment)
{
    const TokenSequence sequence = tokenize_all("// comment\r\nfn main() {\r\n}\r\n");

    ASSERT_EQ(sequence.tokens[0].type, TOKEN_FUNCTION);
    ASSERT_EQ(sequence.tokens[0].line_number, 2);
    ASSERT_EQ(sequence.tokens[1].type, TOKEN_IDENTIFIER);
    ASSERT_EQ(sequence.tokens[1].line_number, 2);
    ASSERT_EQ(sequence.tokens[4].type, TOKEN_LBRACE);
    ASSERT_EQ(sequence.tokens[4].line_number, 2);
    ASSERT_EQ(sequence.tokens[5].type, TOKEN_RBRACE);
    ASSERT_EQ(sequence.tokens[5].line_number, 3);
    ASSERT_EQ(sequence.tokens[6].type, TOKEN_EOF);
}

TEST(test_tokenizer_parameters_and_reference)
{
    const TokenSequence sequence = tokenize_all("fn birthday(user: &User) -> &User {\n    user.age += 1\n}\n");

    const TokenType expected[] = {
        TOKEN_FUNCTION,   TOKEN_IDENTIFIER,  TOKEN_LPAREN,      TOKEN_IDENTIFIER, TOKEN_COLON,  TOKEN_AMP,        TOKEN_IDENTIFIER,
        TOKEN_RPAREN,     TOKEN_ARROW,       TOKEN_AMP,         TOKEN_IDENTIFIER, TOKEN_LBRACE, TOKEN_IDENTIFIER, TOKEN_DOT,
        TOKEN_IDENTIFIER, TOKEN_PLUS_ASSIGN, TOKEN_INT_LITERAL, TOKEN_RBRACE,     TOKEN_EOF,
    };

    assert_token_types(&sequence, expected, sizeof(expected) / sizeof(expected[0]));
}

TEST(test_tokenizer_control_flow)
{
    const TokenSequence sequence = tokenize_all("fn check(items: List<User>) {\n"
                                                "    for item in items {\n"
                                                "        if item.age > 18 {\n"
                                                "            continue\n"
                                                "        } else {\n"
                                                "            break\n"
                                                "        }\n"
                                                "    }\n"
                                                "    while i < 10 {\n"
                                                "        i += 1\n"
                                                "    }\n"
                                                "}\n");

    const TokenType expected[] = {
        TOKEN_FUNCTION,    TOKEN_IDENTIFIER,  TOKEN_LPAREN,      TOKEN_IDENTIFIER, TOKEN_COLON,  TOKEN_IDENTIFIER,  TOKEN_LT,
        TOKEN_IDENTIFIER,  TOKEN_GT,          TOKEN_RPAREN,      TOKEN_LBRACE,     TOKEN_FOR,    TOKEN_IDENTIFIER,  TOKEN_IN,
        TOKEN_IDENTIFIER,  TOKEN_LBRACE,      TOKEN_IF,          TOKEN_IDENTIFIER, TOKEN_DOT,    TOKEN_IDENTIFIER,  TOKEN_GT,
        TOKEN_INT_LITERAL, TOKEN_LBRACE,      TOKEN_CONTINUE,    TOKEN_RBRACE,     TOKEN_ELSE,   TOKEN_LBRACE,      TOKEN_BREAK,
        TOKEN_RBRACE,      TOKEN_RBRACE,      TOKEN_WHILE,       TOKEN_IDENTIFIER, TOKEN_LT,     TOKEN_INT_LITERAL, TOKEN_LBRACE,
        TOKEN_IDENTIFIER,  TOKEN_PLUS_ASSIGN, TOKEN_INT_LITERAL, TOKEN_RBRACE,     TOKEN_RBRACE, TOKEN_EOF,
    };

    assert_token_types(&sequence, expected, sizeof(expected) / sizeof(expected[0]));
}

TEST(test_tokenizer_string_escapes)
{
    const TokenSequence sequence = tokenize_all("println(\"a\\tb\\\"c\")");

    const TokenType expected[] = {
        TOKEN_IDENTIFIER,
        TOKEN_LPAREN,
        TOKEN_STRING_LITERAL,
        TOKEN_RPAREN,
        TOKEN_EOF,
    };

    assert_token_types(&sequence, expected, sizeof(expected) / sizeof(expected[0]));
    ASSERT_EQ(sequence.tokens[2].length, 9);
}

TEST(test_tokenizer_error_propagation)
{
    const TokenSequence sequence = tokenize_all("fn read_config(path: &String) -> Result<Config, IoError> {\n"
                                                "    config := load(path)?\n"
                                                "    return config\n"
                                                "}\n");

    const TokenType expected[] = {
        TOKEN_FUNCTION, TOKEN_IDENTIFIER, TOKEN_LPAREN,     TOKEN_IDENTIFIER, TOKEN_COLON,      TOKEN_AMP,    TOKEN_IDENTIFIER,
        TOKEN_RPAREN,   TOKEN_ARROW,      TOKEN_IDENTIFIER, TOKEN_LT,         TOKEN_IDENTIFIER, TOKEN_COMMA,  TOKEN_IDENTIFIER,
        TOKEN_GT,       TOKEN_LBRACE,     TOKEN_IDENTIFIER, TOKEN_DECLARE,    TOKEN_IDENTIFIER, TOKEN_LPAREN, TOKEN_IDENTIFIER,
        TOKEN_RPAREN,   TOKEN_QUESTION,   TOKEN_RETURN,     TOKEN_IDENTIFIER, TOKEN_RBRACE,     TOKEN_EOF,
    };

    assert_token_types(&sequence, expected, sizeof(expected) / sizeof(expected[0]));
}

RUN_ALL_TESTS()
