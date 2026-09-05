#ifndef WEV_TOKENIZER_H
#define WEV_TOKENIZER_H

#include "token.h"
#include <sys/types.h>

typedef struct Token
{
    TokenType type;
    uint location;
    uint length;
    uint line_number;
    uint col_number;
} Token;

typedef struct Tokenizer
{
    const char* content;
    uint location;
    uint line_number;
    uint col_number;
} Tokenizer;

// Creates a new tokenizer for a string
Tokenizer tokenizer_new(const char* content);

// Returns the next next in the tokenizer and advance the location
Token get_next_token(Tokenizer* tokenizer);

#endif // WEV_TOKENIZER_H
