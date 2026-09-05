#include "parser_internal.h"

#include "token/tokenizer.h"

uint32_t parse_fn_decl(Parser* p)
{
    if (p->failed)
    {
        return parser_failure_node(p);
    }

    if (!parser_expect(p, TOKEN_IDENTIFIER, "expected function name after 'fn'"))
    {
        return parser_failure_node(p);
    }
    const uint32_t name_index = parser_previous_index(p);

    uint32_t first_child = 0;
    bool has_children = false;

    if (!parser_expect(p, TOKEN_LPAREN, "expected '(' after function name"))
    {
        return parser_failure_node(p);
    }

    while (!parser_check(p, TOKEN_RPAREN) && !parser_check(p, TOKEN_EOF))
    {
        const Token token = parser_peek(p);
        parser_error(p, &token, "function parameters are not supported yet");
        return parser_failure_node(p);
    }
    if (!parser_match(p, TOKEN_RPAREN))
    {
        const Token token = parser_peek(p);
        parser_error(p, &token, "expected ')' after function parameters");
        return parser_failure_node(p);
    }

    if (parser_match(p, TOKEN_ARROW))
    {
        const uint32_t return_type = parse_type(p);
        if (!has_children)
        {
            first_child = return_type;
            has_children = true;
        }
    }

    const uint32_t body = parse_block(p);
    if (p->failed)
    {
        return parser_failure_node(p);
    }
    if (!has_children)
    {
        first_child = body;
        has_children = true;
    }

    const uint32_t node = ast_push_fn_decl(p->ast, name_index, false);
    ast_end_children(p->ast, node, has_children ? first_child : node);
    return node;
}