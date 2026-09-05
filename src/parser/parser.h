#ifndef WEV_PARSER_H
#define WEV_PARSER_H

#include <stdbool.h>
#include <sys/types.h>

#include "ast/ast.h"

typedef struct ParseError
{
    uint line;
    uint col;
    const char* message;
} ParseError;

// Parses the whole source into the given AST. Tokenizes the full source first,
// then builds the node tree under an AST_MODULE root.
// On failure, returns false and fills out_error (if given) with the first error.
bool wev_parse_source(const char* source, Ast* ast, ParseError* out_error);

#endif // WEV_PARSER_H