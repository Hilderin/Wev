#include "parser_internal.h"

#include "token/tokenizer.h"

bool wev_parse_source(const char* source, Ast* ast, ParseError* out_error)
{
    ast_init(ast, source);
    Parser parser = {.ast = ast, .source = source};

    Tokenizer tokenizer = tokenizer_new(source);
    for (;;)
    {
        const Token token = get_next_token(&tokenizer);
        ast_push_token(ast, token);
        if (token.type == TOKEN_EOF)
        {
            break;
        }
    }

    parse_program(&parser);

    if (out_error)
    {
        *out_error = parser.error;
    }
    return !parser.failed;
}

Token parser_peek(Parser* p)
{
    return p->ast->tokens[p->cursor];
}

Token parser_previous(Parser* p)
{
    return p->ast->tokens[p->cursor - 1];
}

Token parser_advance(Parser* p)
{
    const Token token = parser_peek(p);
    if (token.type != TOKEN_EOF)
    {
        p->cursor++;
    }
    return token;
}

bool parser_check(Parser* p, const TokenType type)
{
    return parser_peek(p).type == type;
}

bool parser_match(Parser* p, const TokenType type)
{
    if (!parser_check(p, type))
    {
        return false;
    }
    parser_advance(p);
    return true;
}

bool parser_expect(Parser* p, const TokenType type, const char* message)
{
    if (parser_check(p, type))
    {
        parser_advance(p);
        return true;
    }
    const Token token = parser_peek(p);
    parser_error(p, &token, message);
    return false;
}

uint32_t parser_current_index(Parser* p)
{
    return (uint32_t)p->cursor;
}

uint32_t parser_previous_index(Parser* p)
{
    return (uint32_t)(p->cursor - 1);
}

void parser_error(Parser* p, const Token* token, const char* message)
{
    if (p->failed)
    {
        return;
    }
    p->failed = true;
    p->error = (ParseError){.line = token->line_number, .col = token->col_number, .message = message};
}

uint32_t parser_failure_node(Parser* p)
{
    return ast_push_node(p->ast, AST_UNKNOWN, parser_current_index(p));
}

uint32_t parser_intern(Parser* p, const char* text, const uint32_t length)
{
    return ast_intern(p->ast, text, length);
}

uint32_t parse_program(Parser* p)
{
    uint32_t first_child = 0;
    bool has_children = false;

    while (!parser_check(p, TOKEN_EOF) && !p->failed)
    {
        if (parser_match(p, TOKEN_FUNCTION))
        {
            const uint32_t decl = parse_fn_decl(p);
            if (!has_children)
            {
                first_child = decl;
                has_children = true;
            }
            continue;
        }
        const Token token = parser_peek(p);
        parser_error(p, &token, "expected a top-level declaration");
        break;
    }

    const uint32_t module = ast_push_module(p->ast, 0);
    ast_end_children(p->ast, module, has_children ? first_child : module);
    return module;
}