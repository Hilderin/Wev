#include "parser_internal.h"

#include "token/tokenizer.h"

static bool is_primitive_type(const TokenType type)
{
    switch (type)
    {
    case TOKEN_BOOL:
    case TOKEN_I8:
    case TOKEN_I16:
    case TOKEN_I32:
    case TOKEN_I64:
    case TOKEN_U8:
    case TOKEN_U16:
    case TOKEN_U32:
    case TOKEN_U64:
    case TOKEN_ISIZE:
    case TOKEN_USIZE:
    case TOKEN_F32:
    case TOKEN_F64:
    case TOKEN_CHAR:
    case TOKEN_VOID:
        return true;
    default:
        return false;
    }
}

uint32_t parse_type(Parser* p)
{
    if (p->failed)
    {
        return parser_failure_node(p);
    }

    const Token token = parser_peek(p);
    if (is_primitive_type(token.type) || token.type == TOKEN_IDENTIFIER)
    {
        parser_advance(p);
        return ast_push_type(p->ast, parser_previous_index(p), parser_intern(p, p->source + token.location, token.length));
    }
    parser_error(p, &token, "expected a type");
    return parser_failure_node(p);
}