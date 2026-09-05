#ifndef WEV_TOKEN_INFO_H
#define WEV_TOKEN_INFO_H

#include <sys/types.h>

#include "token/token.h"

typedef struct TokenInfo
{
    const char* name;
    const char* str;
    TokenType type;
    uint len;
} TokenInfo;

// Returns the TokenInfo matching str, or NULL if not found.
const TokenInfo* get_token_info(const char* str, uint len);

#endif // WEV_TOKEN_INFO_H
