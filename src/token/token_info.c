#include <stddef.h>
#include <string.h>

#include "token/token_info.h"

#define TOKEN_ENTRY(token_type, token_name, token_str)                                                                                               \
    {                                                                                                                                                \
        .type = (token_type), .name = (token_name), .str = (token_str), .len = sizeof(token_str) - 1                                                 \
    }

static const TokenInfo token_infos[] = {
    [TOKEN_UNKNOWN] = TOKEN_ENTRY(TOKEN_UNKNOWN, "unknown", ""),

    // Keywords
    [TOKEN_FUNCTION] = TOKEN_ENTRY(TOKEN_FUNCTION, "function", "fn"),
    [TOKEN_STRUCT] = TOKEN_ENTRY(TOKEN_STRUCT, "struct", "struct"),
    [TOKEN_IMPL] = TOKEN_ENTRY(TOKEN_IMPL, "impl", "impl"),
    [TOKEN_CONSUMING] = TOKEN_ENTRY(TOKEN_CONSUMING, "consuming", "consuming"),
    [TOKEN_RETURN] = TOKEN_ENTRY(TOKEN_RETURN, "return", "return"),
    [TOKEN_IF] = TOKEN_ENTRY(TOKEN_IF, "if", "if"),
    [TOKEN_ELSE] = TOKEN_ENTRY(TOKEN_ELSE, "else", "else"),
    [TOKEN_WHILE] = TOKEN_ENTRY(TOKEN_WHILE, "while", "while"),
    [TOKEN_FOR] = TOKEN_ENTRY(TOKEN_FOR, "for", "for"),
    [TOKEN_IN] = TOKEN_ENTRY(TOKEN_IN, "in", "in"),
    [TOKEN_BREAK] = TOKEN_ENTRY(TOKEN_BREAK, "break", "break"),
    [TOKEN_CONTINUE] = TOKEN_ENTRY(TOKEN_CONTINUE, "continue", "continue"),
    [TOKEN_MAKE] = TOKEN_ENTRY(TOKEN_MAKE, "make", "make"),
    [TOKEN_ENUM] = TOKEN_ENTRY(TOKEN_ENUM, "enum", "enum"),
    [TOKEN_TRAIT] = TOKEN_ENTRY(TOKEN_TRAIT, "trait", "trait"),
    [TOKEN_EXTERN] = TOKEN_ENTRY(TOKEN_EXTERN, "extern", "extern"),
    [TOKEN_CONST] = TOKEN_ENTRY(TOKEN_CONST, "const", "const"),
    [TOKEN_UNSAFE] = TOKEN_ENTRY(TOKEN_UNSAFE, "unsafe", "unsafe"),
    [TOKEN_INCLUDE] = TOKEN_ENTRY(TOKEN_INCLUDE, "include", "include"),
    [TOKEN_IMPORT] = TOKEN_ENTRY(TOKEN_IMPORT, "import", "import"),
    [TOKEN_AS] = TOKEN_ENTRY(TOKEN_AS, "as", "as"),

    // Primitive types
    [TOKEN_BOOL] = TOKEN_ENTRY(TOKEN_BOOL, "bool", "bool"),
    [TOKEN_I8] = TOKEN_ENTRY(TOKEN_I8, "i8", "i8"),
    [TOKEN_I16] = TOKEN_ENTRY(TOKEN_I16, "i16", "i16"),
    [TOKEN_I32] = TOKEN_ENTRY(TOKEN_I32, "i32", "i32"),
    [TOKEN_I64] = TOKEN_ENTRY(TOKEN_I64, "i64", "i64"),
    [TOKEN_U8] = TOKEN_ENTRY(TOKEN_U8, "u8", "u8"),
    [TOKEN_U16] = TOKEN_ENTRY(TOKEN_U16, "u16", "u16"),
    [TOKEN_U32] = TOKEN_ENTRY(TOKEN_U32, "u32", "u32"),
    [TOKEN_U64] = TOKEN_ENTRY(TOKEN_U64, "u64", "u64"),
    [TOKEN_ISIZE] = TOKEN_ENTRY(TOKEN_ISIZE, "isize", "isize"),
    [TOKEN_USIZE] = TOKEN_ENTRY(TOKEN_USIZE, "usize", "usize"),
    [TOKEN_F32] = TOKEN_ENTRY(TOKEN_F32, "f32", "f32"),
    [TOKEN_F64] = TOKEN_ENTRY(TOKEN_F64, "f64", "f64"),
    [TOKEN_CHAR] = TOKEN_ENTRY(TOKEN_CHAR, "char", "char"),
    [TOKEN_VOID] = TOKEN_ENTRY(TOKEN_VOID, "void", "void"),

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