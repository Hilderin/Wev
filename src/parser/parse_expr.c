#include <stdlib.h>
#include <string.h>

#include "parser_internal.h"

#include "token/tokenizer.h"

static uint64_t parse_int_value(const char* text, const uint32_t length)
{
    uint32_t i = 0;
    uint32_t base = 10;
    if (length >= 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X'))
    {
        base = 16;
        i = 2;
    }
    uint64_t value = 0;
    for (; i < length; i++)
    {
        const char c = text[i];
        uint32_t digit;
        if (c >= '0' && c <= '9')
        {
            digit = (uint32_t)(c - '0');
        }
        else if (c >= 'a' && c <= 'f')
        {
            digit = (uint32_t)(c - 'a') + 10;
        }
        else if (c >= 'A' && c <= 'F')
        {
            digit = (uint32_t)(c - 'A') + 10;
        }
        else
        {
            break;
        }
        value = value * base + digit;
    }
    return value;
}

static double parse_float_value(const char* text, const uint32_t length)
{
    char buffer[64];
    const uint32_t copy_length = length < sizeof(buffer) - 1 ? length : (uint32_t)(sizeof(buffer) - 1);
    memcpy(buffer, text, copy_length);
    buffer[copy_length] = '\0';
    return strtod(buffer, NULL);
}

static uint32_t parse_primary(Parser* p)
{
    if (p->failed)
    {
        return parser_failure_node(p);
    }

    const Token token = parser_peek(p);
    switch (token.type)
    {
    case TOKEN_IDENTIFIER:
    {
        parser_advance(p);
        return ast_push_ident(p->ast, parser_previous_index(p), parser_intern(p, p->source + token.location, token.length));
    }
    case TOKEN_INT_LITERAL:
    {
        parser_advance(p);
        return ast_push_int_literal(p->ast, parser_previous_index(p), parse_int_value(p->source + token.location, token.length));
    }
    case TOKEN_FLOAT_LITERAL:
    {
        parser_advance(p);
        const double value = parse_float_value(p->source + token.location, token.length);
        uint64_t bits;
        memcpy(&bits, &value, sizeof(bits));
        return ast_push_float_literal(p->ast, parser_previous_index(p), bits);
    }
    case TOKEN_STRING_LITERAL:
    {
        parser_advance(p);
        return ast_push_string_literal(p->ast, parser_previous_index(p), parser_intern(p, p->source + token.location + 1, token.length - 2));
    }
    case TOKEN_LPAREN:
    {
        parser_advance(p);
        const uint32_t inner = parse_expression(p);
        if (!parser_match(p, TOKEN_RPAREN))
        {
            const Token token = parser_peek(p);
            parser_error(p, &token, "expected ')' to close the parenthesized expression");
        }
        return inner;
    }
    default:
        parser_error(p, &token, "expected expression");
        return parser_failure_node(p);
    }
}

static uint32_t parse_postfix(Parser* p)
{
    uint32_t expression = parse_primary(p);

    while (!p->failed)
    {
        if (parser_match(p, TOKEN_LPAREN))
        {
            const uint32_t lparen_index = parser_previous_index(p);
            while (!parser_check(p, TOKEN_RPAREN) && !parser_check(p, TOKEN_EOF) && !p->failed)
            {
                parse_expression(p);
                if (!parser_match(p, TOKEN_COMMA))
                {
                    break;
                }
            }
            if (p->failed)
            {
                break;
            }
            if (!parser_match(p, TOKEN_RPAREN))
            {
                const Token token = parser_peek(p);
                parser_error(p, &token, "expected ')' to close the call");
                break;
            }
            const uint32_t node = ast_push_call(p->ast, lparen_index);
            ast_end_children(p->ast, node, expression);
            expression = node;
            continue;
        }
        break;
    }

    return expression;
}

uint32_t parse_expression(Parser* p)
{
    if (p->failed)
    {
        return parser_failure_node(p);
    }
    return parse_postfix(p);
}