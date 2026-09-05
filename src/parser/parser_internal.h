#ifndef WEV_PARSER_INTERNAL_H
#define WEV_PARSER_INTERNAL_H

#include <stdbool.h>
#include <sys/types.h>

#include "parser.h"
#include "token/token.h"

typedef struct Parser
{
    Ast* ast;
    const char* source;
    size_t cursor;
    ParseError error;
    bool failed;
} Parser;

// Token cursor helpers.
Token parser_peek(Parser* p);
Token parser_previous(Parser* p);
Token parser_advance(Parser* p);
bool parser_check(Parser* p, TokenType type);
bool parser_match(Parser* p, TokenType type);
bool parser_expect(Parser* p, TokenType type, const char* message);
uint32_t parser_current_index(Parser* p);
uint32_t parser_previous_index(Parser* p);

// Errors and symbols.
void parser_error(Parser* p, const Token* token, const char* message);
uint32_t parser_failure_node(Parser* p);
uint32_t parser_intern(Parser* p, const char* text, uint32_t length);

// Shared parse functions.
uint32_t parse_program(Parser* p);
uint32_t parse_fn_decl(Parser* p);
uint32_t parse_block(Parser* p);
uint32_t parse_statement(Parser* p);
uint32_t parse_expression(Parser* p);
uint32_t parse_type(Parser* p);

#endif // WEV_PARSER_INTERNAL_H