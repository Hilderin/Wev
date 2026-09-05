#include "ast.h"

#include <stdlib.h>
#include <string.h>

void ast_init(Ast* ast, const char* source)
{
    *ast = (Ast){.source = source};
}

void ast_free(Ast* ast)
{
    free(ast->nodes);
    free(ast->tokens);
    free(ast->symbols);
    *ast = (Ast){0};
}

uint32_t ast_push_token(Ast* ast, const Token token)
{
    if (ast->tokens_count == ast->tokens_capacity)
    {
        const uint32_t new_capacity = ast->tokens_capacity == 0 ? 16 : ast->tokens_capacity * 2;
        Token* const new_tokens = realloc(ast->tokens, (size_t)new_capacity * sizeof(Token));
        if (new_tokens == NULL)
        {
            ast->failed = true;
            return UINT32_MAX;
        }
        ast->tokens = new_tokens;
        ast->tokens_capacity = new_capacity;
    }
    ast->tokens[ast->tokens_count] = token;
    return ast->tokens_count++;
}

uint32_t ast_push_node(Ast* ast, const AstNodeKind kind, const uint32_t token_index)
{
    if (ast->nodes_count == ast->nodes_capacity)
    {
        const uint32_t new_capacity = ast->nodes_capacity == 0 ? 16 : ast->nodes_capacity * 2;
        AstNode* const new_nodes = realloc(ast->nodes, (size_t)new_capacity * sizeof(AstNode));
        if (new_nodes == NULL)
        {
            ast->failed = true;
            return UINT32_MAX;
        }
        ast->nodes = new_nodes;
        ast->nodes_capacity = new_capacity;
    }
    const uint32_t index = ast->nodes_count++;
    ast->nodes[index] = (AstNode){.kind = kind, .token_index = token_index};
    return index;
}

uint32_t ast_begin_children(Ast* ast)
{
    return ast->nodes_count;
}

void ast_end_children(Ast* ast, const uint32_t node_index, const uint32_t start)
{
    ast->nodes[node_index].first_child = start;
    ast->nodes[node_index].child_count = node_index - start;
}

uint32_t ast_intern(Ast* ast, const char* text, const uint32_t length)
{
    for (size_t i = 0; i < ast->symbols_count; i++)
    {
        const AstSymbol* symbol = &ast->symbols[i];
        if (symbol->length != length)
        {
            continue;
        }
        if (memcmp(ast->source + symbol->offset, text, length) == 0)
        {
            return (uint32_t)i;
        }
    }

    if (ast->symbols_count == ast->symbols_capacity)
    {
        const uint32_t new_capacity = ast->symbols_capacity == 0 ? 16 : ast->symbols_capacity * 2;
        AstSymbol* const new_symbols = realloc(ast->symbols, (size_t)new_capacity * sizeof(AstSymbol));
        if (new_symbols == NULL)
        {
            ast->failed = true;
            return UINT32_MAX;
        }
        ast->symbols = new_symbols;
        ast->symbols_capacity = new_capacity;
    }
    ast->symbols[ast->symbols_count] = (AstSymbol){.offset = (uint32_t)(text - ast->source), .length = length};
    return ast->symbols_count++;
}

uint32_t ast_push_module(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_MODULE, token_index);
}

uint32_t ast_push_include(Ast* ast, const uint32_t token_index, const bool is_system)
{
    const uint32_t node_index = ast_push_node(ast, AST_INCLUDE, token_index);
    if (is_system)
    {
        ast->nodes[node_index].payload.flags |= AST_FLAG_IS_SYSTEM;
    }
    return node_index;
}

uint32_t ast_push_import(Ast* ast, const uint32_t token_index, const uint32_t symbol_id)
{
    const uint32_t node_index = ast_push_node(ast, AST_IMPORT, token_index);
    ast->nodes[node_index].payload.symbol_id = symbol_id;
    return node_index;
}

uint32_t ast_push_fn_decl(Ast* ast, const uint32_t token_index, const bool consuming)
{
    const uint32_t node_index = ast_push_node(ast, AST_FN_DECL, token_index);
    if (consuming)
    {
        ast->nodes[node_index].payload.flags |= AST_FLAG_CONSUMING;
    }
    return node_index;
}

uint32_t ast_push_struct_decl(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_STRUCT_DECL, token_index);
}

uint32_t ast_push_enum_decl(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_ENUM_DECL, token_index);
}

uint32_t ast_push_trait_decl(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_TRAIT_DECL, token_index);
}

uint32_t ast_push_impl(Ast* ast, const uint32_t token_index, const bool is_trait_impl)
{
    const uint32_t node_index = ast_push_node(ast, AST_IMPL, token_index);
    if (is_trait_impl)
    {
        ast->nodes[node_index].payload.flags |= AST_FLAG_IS_TRAIT_IMPL;
    }
    return node_index;
}

uint32_t ast_push_const_decl(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_CONST_DECL, token_index);
}

uint32_t ast_push_extern_block(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_EXTERN_BLOCK, token_index);
}

uint32_t ast_push_type(Ast* ast, const uint32_t token_index, const uint32_t symbol_id)
{
    const uint32_t node_index = ast_push_node(ast, AST_TYPE, token_index);
    ast->nodes[node_index].payload.symbol_id = symbol_id;
    return node_index;
}

uint32_t ast_push_heap_type(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_HEAP_TYPE, token_index);
}

uint32_t ast_push_ptr_type(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_PTR_TYPE, token_index);
}

uint32_t ast_push_ref_type(Ast* ast, const uint32_t token_index, const bool is_mut)
{
    const uint32_t node_index = ast_push_node(ast, AST_REF_TYPE, token_index);
    if (is_mut)
    {
        ast->nodes[node_index].payload.flags |= AST_FLAG_IS_MUT;
    }
    return node_index;
}

uint32_t ast_push_generic_args(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_GENERIC_ARGS, token_index);
}

uint32_t ast_push_block(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_BLOCK, token_index);
}

uint32_t ast_push_var_decl(Ast* ast, const uint32_t token_index, const bool has_explicit_type)
{
    const uint32_t node_index = ast_push_node(ast, AST_VAR_DECL, token_index);
    if (has_explicit_type)
    {
        ast->nodes[node_index].payload.flags |= AST_FLAG_HAS_EXPLICIT_TYPE;
    }
    return node_index;
}

uint32_t ast_push_assign(Ast* ast, const uint32_t token_index, const TokenType op)
{
    const uint32_t node_index = ast_push_node(ast, AST_ASSIGN, token_index);
    ast->nodes[node_index].payload.op = op;
    return node_index;
}

uint32_t ast_push_return(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_RETURN, token_index);
}

uint32_t ast_push_if(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_IF, token_index);
}

uint32_t ast_push_while(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_WHILE, token_index);
}

uint32_t ast_push_for(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_FOR, token_index);
}

uint32_t ast_push_break(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_BREAK, token_index);
}

uint32_t ast_push_continue(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_CONTINUE, token_index);
}

uint32_t ast_push_ident(Ast* ast, const uint32_t token_index, const uint32_t symbol_id)
{
    const uint32_t node_index = ast_push_node(ast, AST_IDENT, token_index);
    ast->nodes[node_index].payload.symbol_id = symbol_id;
    return node_index;
}

uint32_t ast_push_int_literal(Ast* ast, const uint32_t token_index, const uint64_t value)
{
    const uint32_t node_index = ast_push_node(ast, AST_INT_LITERAL, token_index);
    ast->nodes[node_index].payload.int_value = value;
    return node_index;
}

uint32_t ast_push_float_literal(Ast* ast, const uint32_t token_index, const uint64_t float_bits)
{
    const uint32_t node_index = ast_push_node(ast, AST_FLOAT_LITERAL, token_index);
    ast->nodes[node_index].payload.float_bits = float_bits;
    return node_index;
}

uint32_t ast_push_string_literal(Ast* ast, const uint32_t token_index, const uint32_t symbol_id)
{
    const uint32_t node_index = ast_push_node(ast, AST_STRING_LITERAL, token_index);
    ast->nodes[node_index].payload.symbol_id = symbol_id;
    return node_index;
}

uint32_t ast_push_char_literal(Ast* ast, const uint32_t token_index, const uint32_t char_value)
{
    const uint32_t node_index = ast_push_node(ast, AST_CHAR_LITERAL, token_index);
    ast->nodes[node_index].payload.char_value = char_value;
    return node_index;
}

uint32_t ast_push_call(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_CALL, token_index);
}

uint32_t ast_push_method_call(Ast* ast, const uint32_t token_index, const uint32_t method_symbol_id)
{
    const uint32_t node_index = ast_push_node(ast, AST_METHOD_CALL, token_index);
    ast->nodes[node_index].payload.symbol_id = method_symbol_id;
    return node_index;
}

uint32_t ast_push_struct_literal(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_STRUCT_LITERAL, token_index);
}

uint32_t ast_push_binary_op(Ast* ast, const uint32_t token_index, const TokenType op)
{
    const uint32_t node_index = ast_push_node(ast, AST_BINARY_OP, token_index);
    ast->nodes[node_index].payload.op = op;
    return node_index;
}

uint32_t ast_push_unary_op(Ast* ast, const uint32_t token_index, const TokenType op)
{
    const uint32_t node_index = ast_push_node(ast, AST_UNARY_OP, token_index);
    ast->nodes[node_index].payload.op = op;
    return node_index;
}

uint32_t ast_push_index(Ast* ast, const uint32_t token_index)
{
    return ast_push_node(ast, AST_INDEX, token_index);
}

uint32_t ast_push_field(Ast* ast, const uint32_t token_index, const uint32_t field_symbol_id)
{
    const uint32_t node_index = ast_push_node(ast, AST_FIELD, token_index);
    ast->nodes[node_index].payload.symbol_id = field_symbol_id;
    return node_index;
}