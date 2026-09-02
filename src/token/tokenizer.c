#include "tokenizer.h"

#include <stdbool.h>

#include "token_info.h"

// Create a token from the tokenizer,
static Token create_token(const Tokenizer* tokenizer,
                          const TokenType type,
                          const uint location,
                          const uint size,
                          const uint line_number,
                          const uint col_number)
{
    return (Token){
        .type = type,
        .str_ptr = tokenizer->content + location,
        .location = location,
        .length = size,
        .line_number = line_number,
        .col_number = col_number,
    };
}

// Advance one character and update the line/column tracking.
// '\n', '\r' and '\r\n' each count as a single line break.
static void advance(Tokenizer* tokenizer)
{
    const char next_char = tokenizer->content[tokenizer->location];

    if (next_char == '\n')
    {
        tokenizer->line_number++;
        tokenizer->col_number = 1;
        tokenizer->location++;
    }
    else if (next_char == '\r')
    {
        tokenizer->line_number++;
        tokenizer->col_number = 1;
        tokenizer->location++;
        if (tokenizer->content[tokenizer->location] == '\n')
        {
            tokenizer->location++;
        }
    }
    else
    {
        tokenizer->col_number++;
        tokenizer->location++;
    }
}

// Character classification helpers.
static bool is_identifier_start(const char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_identifier_continue(const char c)
{
    return is_identifier_start(c) || (c >= '0' && c <= '9');
}

static bool is_digit(const char c)
{
    return c >= '0' && c <= '9';
}

static bool is_hex_digit(const char c)
{
    return is_digit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

// Skip whitespace, line comments and block comments.
static void move_to_next_token_start(Tokenizer* tokenizer)
{
    while (true)
    {
        const char next_char = tokenizer->content[tokenizer->location];

        switch (next_char)
        {
        case '\r':
        case ' ':
        case '\t':
        case '\n':
            advance(tokenizer);
            continue;
        default:
            break;
        }

        if (next_char == '/' && tokenizer->content[tokenizer->location + 1] == '/')
        {
            while (tokenizer->content[tokenizer->location] != '\0' && tokenizer->content[tokenizer->location] != '\n' &&
                   tokenizer->content[tokenizer->location] != '\r')
            {
                advance(tokenizer);
            }
            continue;
        }

        if (next_char == '/' && tokenizer->content[tokenizer->location + 1] == '*')
        {
            advance(tokenizer);
            advance(tokenizer);
            while (tokenizer->content[tokenizer->location] != '\0')
            {
                if (tokenizer->content[tokenizer->location] == '*' && tokenizer->content[tokenizer->location + 1] == '/')
                {
                    advance(tokenizer);
                    advance(tokenizer);
                    break;
                }
                advance(tokenizer);
            }
            continue;
        }

        return;
    }
}

// Scan an identifier or keyword token.
static Token scan_identifier(Tokenizer* tokenizer, const uint start_location, const uint line_number, const uint col_number)
{
    while (is_identifier_continue(tokenizer->content[tokenizer->location]))
    {
        advance(tokenizer);
    }

    const uint length = tokenizer->location - start_location;
    const TokenInfo* token_info = get_token_info(tokenizer->content + start_location, length);

    if (token_info)
    {
        return create_token(tokenizer, token_info->type, start_location, length, line_number, col_number);
    }

    return create_token(tokenizer, TOKEN_IDENTIFIER, start_location, length, line_number, col_number);
}

// Scan an integer or float literal token.
static Token scan_number(Tokenizer* tokenizer, const uint start_location, const uint line_number, const uint col_number)
{
    bool is_float = false;

    if (tokenizer->content[tokenizer->location] == '0' &&
        (tokenizer->content[tokenizer->location + 1] == 'x' || tokenizer->content[tokenizer->location + 1] == 'X'))
    {
        advance(tokenizer);
        advance(tokenizer);
        while (is_hex_digit(tokenizer->content[tokenizer->location]))
        {
            advance(tokenizer);
        }
        return create_token(tokenizer, TOKEN_INT_LITERAL, start_location, tokenizer->location - start_location, line_number, col_number);
    }

    while (is_digit(tokenizer->content[tokenizer->location]))
    {
        advance(tokenizer);
    }

    if (tokenizer->content[tokenizer->location] == '.' && is_digit(tokenizer->content[tokenizer->location + 1]))
    {
        is_float = true;
        advance(tokenizer);
        while (is_digit(tokenizer->content[tokenizer->location]))
        {
            advance(tokenizer);
        }
    }

    if (tokenizer->content[tokenizer->location] == 'e' || tokenizer->content[tokenizer->location] == 'E')
    {
        const char next_char = tokenizer->content[tokenizer->location + 1];
        if (is_digit(next_char) || ((next_char == '+' || next_char == '-') && is_digit(tokenizer->content[tokenizer->location + 2])))
        {
            is_float = true;
            advance(tokenizer);
            if (tokenizer->content[tokenizer->location] == '+' || tokenizer->content[tokenizer->location] == '-')
            {
                advance(tokenizer);
            }
            while (is_digit(tokenizer->content[tokenizer->location]))
            {
                advance(tokenizer);
            }
        }
    }

    return create_token(tokenizer,
                        is_float ? TOKEN_FLOAT_LITERAL : TOKEN_INT_LITERAL,
                        start_location,
                        tokenizer->location - start_location,
                        line_number,
                        col_number);
}

// Scan a string literal token, including the surrounding quotes.
static Token scan_string(Tokenizer* tokenizer, const uint start_location, const uint line_number, const uint col_number)
{
    advance(tokenizer); // opening quote

    while (true)
    {
        const char next_char = tokenizer->content[tokenizer->location];

        if (next_char == '"')
        {
            advance(tokenizer); // closing quote
            break;
        }
        if (next_char == '\0' || next_char == '\n' || next_char == '\r')
        {
            break; // unterminated string literal
        }
        if (next_char == '\\')
        {
            advance(tokenizer);
            if (tokenizer->content[tokenizer->location] != '\0')
            {
                advance(tokenizer);
            }
            continue;
        }
        advance(tokenizer);
    }

    return create_token(tokenizer, TOKEN_STRING_LITERAL, start_location, tokenizer->location - start_location, line_number, col_number);
}

// Scan a character literal token, including the surrounding quotes.
static Token scan_char(Tokenizer* tokenizer, const uint start_location, const uint line_number, const uint col_number)
{
    advance(tokenizer); // opening quote

    if (tokenizer->content[tokenizer->location] == '\\')
    {
        advance(tokenizer);
        if (tokenizer->content[tokenizer->location] != '\0')
        {
            advance(tokenizer);
        }
    }
    else if (tokenizer->content[tokenizer->location] != '\0' && tokenizer->content[tokenizer->location] != '\n' &&
             tokenizer->content[tokenizer->location] != '\r')
    {
        advance(tokenizer);
    }

    if (tokenizer->content[tokenizer->location] == '\'')
    {
        advance(tokenizer); // closing quote
    }

    return create_token(tokenizer, TOKEN_CHAR_LITERAL, start_location, tokenizer->location - start_location, line_number, col_number);
}

// Advance the tokenizer for a punctuation or operator token of a fixed length.
static Token make_punctuation_token(Tokenizer* tokenizer,
                                    const TokenType type,
                                    const uint start_location,
                                    const uint length,
                                    const uint line_number,
                                    const uint col_number)
{
    for (uint i = 0; i < length; i++)
    {
        advance(tokenizer);
    }
    return create_token(tokenizer, type, start_location, length, line_number, col_number);
}

// Scan a punctuation or operator token.
static Token scan_punctuation(Tokenizer* tokenizer, const uint start_location, const uint line_number, const uint col_number)
{
    const char next_char = tokenizer->content[tokenizer->location];
    const char lookahead = tokenizer->content[tokenizer->location + 1];

    switch (next_char)
    {
    case '{':
        return make_punctuation_token(tokenizer, TOKEN_LBRACE, start_location, 1, line_number, col_number);
    case '}':
        return make_punctuation_token(tokenizer, TOKEN_RBRACE, start_location, 1, line_number, col_number);
    case '(':
        return make_punctuation_token(tokenizer, TOKEN_LPAREN, start_location, 1, line_number, col_number);
    case ')':
        return make_punctuation_token(tokenizer, TOKEN_RPAREN, start_location, 1, line_number, col_number);
    case '[':
        return make_punctuation_token(tokenizer, TOKEN_LBRACKET, start_location, 1, line_number, col_number);
    case ']':
        return make_punctuation_token(tokenizer, TOKEN_RBRACKET, start_location, 1, line_number, col_number);
    case ',':
        return make_punctuation_token(tokenizer, TOKEN_COMMA, start_location, 1, line_number, col_number);
    case ';':
        return make_punctuation_token(tokenizer, TOKEN_SEMICOLON, start_location, 1, line_number, col_number);
    case '.':
        return make_punctuation_token(tokenizer, TOKEN_DOT, start_location, 1, line_number, col_number);
    case '?':
        return make_punctuation_token(tokenizer, TOKEN_QUESTION, start_location, 1, line_number, col_number);
    case ':':
        return make_punctuation_token(tokenizer,
                                      lookahead == '=' ? TOKEN_DECLARE : TOKEN_COLON,
                                      start_location,
                                      lookahead == '=' ? 2 : 1,
                                      line_number,
                                      col_number);
    case '=':
        return make_punctuation_token(tokenizer,
                                      lookahead == '=' ? TOKEN_EQ : TOKEN_ASSIGN,
                                      start_location,
                                      lookahead == '=' ? 2 : 1,
                                      line_number,
                                      col_number);
    case '+':
        return make_punctuation_token(tokenizer,
                                      lookahead == '=' ? TOKEN_PLUS_ASSIGN : TOKEN_PLUS,
                                      start_location,
                                      lookahead == '=' ? 2 : 1,
                                      line_number,
                                      col_number);
    case '-':
        if (lookahead == '=') return make_punctuation_token(tokenizer, TOKEN_MINUS_ASSIGN, start_location, 2, line_number, col_number);
        if (lookahead == '>') return make_punctuation_token(tokenizer, TOKEN_ARROW, start_location, 2, line_number, col_number);
        return make_punctuation_token(tokenizer, TOKEN_MINUS, start_location, 1, line_number, col_number);
    case '*':
        return make_punctuation_token(tokenizer,
                                      lookahead == '=' ? TOKEN_STAR_ASSIGN : TOKEN_STAR,
                                      start_location,
                                      lookahead == '=' ? 2 : 1,
                                      line_number,
                                      col_number);
    case '/':
        return make_punctuation_token(tokenizer,
                                      lookahead == '=' ? TOKEN_SLASH_ASSIGN : TOKEN_SLASH,
                                      start_location,
                                      lookahead == '=' ? 2 : 1,
                                      line_number,
                                      col_number);
    case '%':
        return make_punctuation_token(tokenizer,
                                      lookahead == '=' ? TOKEN_PERCENT_ASSIGN : TOKEN_PERCENT,
                                      start_location,
                                      lookahead == '=' ? 2 : 1,
                                      line_number,
                                      col_number);
    case '&':
        return make_punctuation_token(tokenizer,
                                      lookahead == '&' ? TOKEN_AND : TOKEN_AMP,
                                      start_location,
                                      lookahead == '&' ? 2 : 1,
                                      line_number,
                                      col_number);
    case '|':
        return make_punctuation_token(tokenizer,
                                      lookahead == '|' ? TOKEN_OR : TOKEN_PIPE,
                                      start_location,
                                      lookahead == '|' ? 2 : 1,
                                      line_number,
                                      col_number);
    case '~':
        return make_punctuation_token(tokenizer, TOKEN_TILDE, start_location, 1, line_number, col_number);
    case '!':
        return make_punctuation_token(tokenizer,
                                      lookahead == '=' ? TOKEN_NE : TOKEN_NOT,
                                      start_location,
                                      lookahead == '=' ? 2 : 1,
                                      line_number,
                                      col_number);
    case '<':
        return make_punctuation_token(tokenizer,
                                      lookahead == '=' ? TOKEN_LE : TOKEN_LT,
                                      start_location,
                                      lookahead == '=' ? 2 : 1,
                                      line_number,
                                      col_number);
    case '>':
        return make_punctuation_token(tokenizer,
                                      lookahead == '=' ? TOKEN_GE : TOKEN_GT,
                                      start_location,
                                      lookahead == '=' ? 2 : 1,
                                      line_number,
                                      col_number);
    default:
        return make_punctuation_token(tokenizer, TOKEN_UNKNOWN, start_location, 1, line_number, col_number);
    }
}

// Create a new tokenizer.
Tokenizer tokenizer_new(const char* content)
{
    return (Tokenizer){.content = content, .line_number = 1, .col_number = 1, .location = 0};
}

// Get the next token.
Token get_next_token(Tokenizer* tokenizer)
{
    move_to_next_token_start(tokenizer);

    const char next_char = tokenizer->content[tokenizer->location];

    if (next_char == '\0')
    {
        return create_token(tokenizer, TOKEN_EOF, tokenizer->location, 0, tokenizer->line_number, tokenizer->col_number);
    }

    const uint start_location = tokenizer->location;
    const uint line_number = tokenizer->line_number;
    const uint col_number = tokenizer->col_number;

    if (is_identifier_start(next_char))
    {
        return scan_identifier(tokenizer, start_location, line_number, col_number);
    }
    if (is_digit(next_char))
    {
        return scan_number(tokenizer, start_location, line_number, col_number);
    }
    if (next_char == '"')
    {
        return scan_string(tokenizer, start_location, line_number, col_number);
    }
    if (next_char == '\'')
    {
        return scan_char(tokenizer, start_location, line_number, col_number);
    }

    return scan_punctuation(tokenizer, start_location, line_number, col_number);
}