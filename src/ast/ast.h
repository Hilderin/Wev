#ifndef WEV_AST_H
#define WEV_AST_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "token/tokenizer.h"

typedef enum AstNodeKind
{
    AST_UNKNOWN,

    // Declarations
    AST_MODULE,
    AST_INCLUDE,
    AST_FN_DECL,
    AST_STRUCT_DECL,
    AST_ENUM_DECL,
    AST_TRAIT_DECL,
    AST_IMPL,
    AST_CONST_DECL,
    AST_EXTERN_BLOCK,

    // Types
    AST_TYPE,
    AST_HEAP_TYPE,
    AST_PTR_TYPE,
    AST_REF_TYPE,
    AST_GENERIC_ARGS,

    // Statements
    AST_BLOCK,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_RETURN,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_BREAK,
    AST_CONTINUE,

    // Expressions
    AST_IDENT,
    AST_INT_LITERAL,
    AST_FLOAT_LITERAL,
    AST_STRING_LITERAL,
    AST_CHAR_LITERAL,
    AST_CALL,
    AST_METHOD_CALL,
    AST_STRUCT_LITERAL,
    AST_BINARY_OP,
    AST_UNARY_OP,
    AST_INDEX,
    AST_FIELD,
} AstNodeKind;

typedef struct AstNode
{
    uint32_t kind;
    uint32_t token_index;
    uint32_t first_child;
    uint32_t child_count;
    union
    {
        uint64_t int_value;
        uint64_t float_bits;
        uint32_t symbol_id;
        uint32_t char_value;
        TokenType op;
        uint32_t flags;
    } payload;
} AstNode;

typedef struct Ast
{
    AstNode* nodes;
    size_t nodes_count;
    size_t nodes_capacity;
    Token* tokens;
    size_t tokens_count;
    size_t tokens_capacity;
    const char* source;
} Ast;

#define AST_FLAG_IS_SYSTEM (1u << 0)         // AST_INCLUDE
#define AST_FLAG_CONSUMING (1u << 1)         // AST_FN_DECL
#define AST_FLAG_IS_TRAIT_IMPL (1u << 2)     // AST_IMPL
#define AST_FLAG_HAS_EXPLICIT_TYPE (1u << 3) // AST_VAR_DECL
#define AST_FLAG_IS_MUT (1u << 4)            // AST_REF_TYPE

// Initializes an empty AST over the given source.
void ast_init(Ast* ast, const char* source);

// Frees all arrays owned by the AST.
void ast_free(Ast* ast);

// Appends a token and returns its index.
uint32_t ast_push_token(Ast* ast, Token token);

// Appends a node and returns its index.
uint32_t ast_push_node(Ast* ast, AstNodeKind kind, uint32_t token_index);

// Records the start of a child range before parsing children.
uint32_t ast_begin_children(Ast* ast);

// Writes the child range [start, node_index) into the given node.
void ast_end_children(Ast* ast, uint32_t node_index, uint32_t start);

// Declaration nodes.
uint32_t ast_push_module(Ast* ast, uint32_t token_index);
uint32_t ast_push_include(Ast* ast, uint32_t token_index, bool is_system);
uint32_t ast_push_fn_decl(Ast* ast, uint32_t token_index, bool consuming);
uint32_t ast_push_struct_decl(Ast* ast, uint32_t token_index);
uint32_t ast_push_enum_decl(Ast* ast, uint32_t token_index);
uint32_t ast_push_trait_decl(Ast* ast, uint32_t token_index);
uint32_t ast_push_impl(Ast* ast, uint32_t token_index, bool is_trait_impl);
uint32_t ast_push_const_decl(Ast* ast, uint32_t token_index);
uint32_t ast_push_extern_block(Ast* ast, uint32_t token_index);

// Type nodes.
uint32_t ast_push_type(Ast* ast, uint32_t token_index, uint32_t symbol_id);
uint32_t ast_push_heap_type(Ast* ast, uint32_t token_index);
uint32_t ast_push_ptr_type(Ast* ast, uint32_t token_index);
uint32_t ast_push_ref_type(Ast* ast, uint32_t token_index, bool is_mut);
uint32_t ast_push_generic_args(Ast* ast, uint32_t token_index);

// Statement nodes.
uint32_t ast_push_block(Ast* ast, uint32_t token_index);
uint32_t ast_push_var_decl(Ast* ast, uint32_t token_index, bool has_explicit_type);
uint32_t ast_push_assign(Ast* ast, uint32_t token_index, TokenType op);
uint32_t ast_push_return(Ast* ast, uint32_t token_index);
uint32_t ast_push_if(Ast* ast, uint32_t token_index);
uint32_t ast_push_while(Ast* ast, uint32_t token_index);
uint32_t ast_push_for(Ast* ast, uint32_t token_index);
uint32_t ast_push_break(Ast* ast, uint32_t token_index);
uint32_t ast_push_continue(Ast* ast, uint32_t token_index);

// Expression nodes.
uint32_t ast_push_ident(Ast* ast, uint32_t token_index, uint32_t symbol_id);
uint32_t ast_push_int_literal(Ast* ast, uint32_t token_index, uint64_t value);
uint32_t ast_push_float_literal(Ast* ast, uint32_t token_index, uint64_t float_bits);
uint32_t ast_push_string_literal(Ast* ast, uint32_t token_index, uint32_t symbol_id);
uint32_t ast_push_char_literal(Ast* ast, uint32_t token_index, uint32_t char_value);
uint32_t ast_push_call(Ast* ast, uint32_t token_index);
uint32_t ast_push_method_call(Ast* ast, uint32_t token_index, uint32_t method_symbol_id);
uint32_t ast_push_struct_literal(Ast* ast, uint32_t token_index);
uint32_t ast_push_binary_op(Ast* ast, uint32_t token_index, TokenType op);
uint32_t ast_push_unary_op(Ast* ast, uint32_t token_index, TokenType op);
uint32_t ast_push_index(Ast* ast, uint32_t token_index);
uint32_t ast_push_field(Ast* ast, uint32_t token_index, uint32_t field_symbol_id);

#endif // WEV_AST_H