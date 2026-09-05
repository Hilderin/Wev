#include "parser_internal.h"

#include "token/tokenizer.h"

uint32_t parse_block(Parser* p)
{
    if (p->failed)
    {
        return parser_failure_node(p);
    }

    if (!parser_match(p, TOKEN_LBRACE))
    {
        const Token token = parser_peek(p);
        parser_error(p, &token, "expected '{' to start a block");
        return parser_failure_node(p);
    }
    const uint32_t lbrace_index = parser_previous_index(p);

    uint32_t first_child = 0;
    bool has_children = false;

    while (!parser_check(p, TOKEN_RBRACE) && !parser_check(p, TOKEN_EOF) && !p->failed)
    {
        const uint32_t statement = parse_statement(p);
        if (!has_children)
        {
            first_child = statement;
            has_children = true;
        }
    }
    if (!parser_match(p, TOKEN_RBRACE))
    {
        const Token token = parser_peek(p);
        parser_error(p, &token, "expected '}' to close the block");
    }

    const uint32_t node = ast_push_block(p->ast, lbrace_index);
    ast_end_children(p->ast, node, has_children ? first_child : node);
    return node;
}

uint32_t parse_statement(Parser* p)
{
    if (p->failed)
    {
        return parser_failure_node(p);
    }
    const uint32_t expression = parse_expression(p);
    parser_match(p, TOKEN_SEMICOLON);
    return expression;
}