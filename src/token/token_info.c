#include <stddef.h>
#include <string.h>

#include "token/token_info.h"

#define TOKEN_ENTRY(token_type, token_name, token_str) {.type = token_type, .name = token_name, .str = token_str, .len = sizeof(token_str) - 1}

static const TokenInfo token_infos[] = {
    [TOKEN_UNKNOWN] = TOKEN_ENTRY(TOKEN_UNKNOWN, "unknown", ""),
    [TOKEN_FUNCTION] = TOKEN_ENTRY(TOKEN_FUNCTION, "function", "fn"),
    [TOKEN_EOF] = TOKEN_ENTRY(TOKEN_EOF, "eof", ""),
};

static const uint token_info_count = sizeof(token_infos) / sizeof(token_infos[0]);

const TokenInfo* get_token_info(const char* str, const uint len)
{
    if (len == 0) return NULL;

    for (uint i = 1; i < token_info_count; i++)
    {
        if (token_infos[i].len == len && memcmp(token_infos[i].str, str, len) == 0)
        {
            return &token_infos[i];
        }
    }
    return NULL;
}
