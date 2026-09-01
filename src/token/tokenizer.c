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

// Move the tokenizer to the next character that is not a whitespace.
static void move_to_next_non_whitespace(Tokenizer* tokenizer)
{
    while (true)
    {
        const char next_char = tokenizer->content[tokenizer->location];

        switch (next_char)
        {
        case '\r':
        case ' ':
        case '\t':
            // Whitespace
            tokenizer->location++;
            tokenizer->col_number++;
            break;
        case '\n':
            tokenizer->location++;
            tokenizer->col_number = 1;
            tokenizer->line_number++;
            break;
        default:
            return;
        }
    }
}

// Move the tokenizer to the next separator token.
static void move_to_next_separator(Tokenizer* tokenizer)
{
    while (true)
    {
        const char next_char = tokenizer->content[tokenizer->location];

        switch (next_char)
        {
        case '\r':
        case ' ':
        case '\t':
        case '{':
        case '}':
        case '(':
        case ')':
        case ',':
        case ';':
        case '+':
        case '-':
        case '/':
        case '*':
        case '%':
        case '&':
        case '!':
        case '|':
        case '~':
        case '<':
        case '>':
        case '=':
        case '\0':
            return;
        default:
            tokenizer->location++;
            tokenizer->col_number++;
            break;
        }
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
    move_to_next_non_whitespace(tokenizer);

    const char next_char = tokenizer->content[tokenizer->location];

    if (next_char == '\0')
    {
        return create_token(tokenizer, TOKEN_EOF, tokenizer->location, 0, tokenizer->line_number, tokenizer->col_number);
    }

    const uint start_location = tokenizer->location;
    const uint line_number = tokenizer->line_number;
    const uint col_number = tokenizer->col_number;

    tokenizer->location++;
    tokenizer->col_number++;
    move_to_next_separator(tokenizer);

    const TokenInfo* token_info = get_token_info(tokenizer->content + start_location, tokenizer->location - start_location);

    if (token_info)
    {
        return create_token(tokenizer, token_info->type, start_location, tokenizer->location - start_location, line_number, col_number);
    }

    return create_token(tokenizer, TOKEN_UNKNOWN, start_location, tokenizer->location - start_location, line_number, col_number);
}
