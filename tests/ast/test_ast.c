#include <string.h>

#include "ast/ast.h"
#include "test_util.h"
#include "token/tokenizer.h"

// Asserts the kind and token_index of a node.
static void assert_node(const Ast* ast, const uint32_t node_index, const AstNodeKind kind, const uint32_t token_index)
{
    ASSERT_EQ(ast->nodes[node_index].kind, kind);
    ASSERT_EQ(ast->nodes[node_index].token_index, token_index);
}

TEST(test_ast_init)
{
    Ast ast;
    ast_init(&ast, "42");

    ASSERT_STR_EQ(ast.source, "42");
    ASSERT_EQ(ast.nodes_count, 0);
    ASSERT_EQ(ast.tokens_count, 0);

    ast_free(&ast);
}

TEST(test_ast_push_node)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t first = ast_push_node(&ast, AST_INT_LITERAL, 0);
    const uint32_t second = ast_push_node(&ast, AST_INT_LITERAL, 1);

    ASSERT_EQ(first, 0);
    ASSERT_EQ(second, 1);
    ASSERT_EQ(ast.nodes_count, 2);
    assert_node(&ast, first, AST_INT_LITERAL, 0);
    assert_node(&ast, second, AST_INT_LITERAL, 1);
    ASSERT_EQ(ast.nodes[first].child_count, 0);

    ast_free(&ast);
}

TEST(test_ast_push_node_growth)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t first = ast_push_node(&ast, AST_INT_LITERAL, 0);
    for (uint32_t i = 1; i < 1000; i++)
    {
        ast_push_node(&ast, AST_INT_LITERAL, i);
    }

    ASSERT_EQ(ast.nodes_count, 1000);
    assert_node(&ast, first, AST_INT_LITERAL, 0);
    assert_node(&ast, 999, AST_INT_LITERAL, 999);

    ast_free(&ast);
}

TEST(test_ast_push_token)
{
    Ast ast;
    ast_init(&ast, "42");
    Tokenizer tokenizer = tokenizer_new("42");

    const Token token = get_next_token(&tokenizer);
    const uint32_t token_index = ast_push_token(&ast, token);

    ASSERT_EQ(token_index, 0);
    ASSERT_EQ(ast.tokens_count, 1);
    ASSERT_EQ(ast.tokens[token_index].type, TOKEN_INT_LITERAL);
    ASSERT_TRUE(memcmp(ast.source + ast.tokens[token_index].location, "42", ast.tokens[token_index].length) == 0);

    ast_free(&ast);
}

TEST(test_ast_children_range)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t start = ast_begin_children(&ast);
    const uint32_t child_a = ast_push_node(&ast, AST_INT_LITERAL, 0);
    const uint32_t child_b = ast_push_node(&ast, AST_INT_LITERAL, 1);
    const uint32_t parent = ast_push_node(&ast, AST_INT_LITERAL, 2);
    ast_end_children(&ast, parent, start);

    ASSERT_EQ(parent, 2);
    ASSERT_EQ(ast.nodes[parent].first_child, 0);
    ASSERT_EQ(ast.nodes[parent].child_count, 2);
    ASSERT_EQ(child_a, ast.nodes[parent].first_child);
    ASSERT_EQ(child_b, ast.nodes[parent].first_child + 1);

    ast_free(&ast);
}

TEST(test_ast_push_module)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_module(&ast, 3);

    assert_node(&ast, node_index, AST_MODULE, 3);

    ast_free(&ast);
}

TEST(test_ast_push_include)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t system = ast_push_include(&ast, 0, true);
    const uint32_t local = ast_push_include(&ast, 1, false);

    assert_node(&ast, system, AST_INCLUDE, 0);
    assert_node(&ast, local, AST_INCLUDE, 1);
    ASSERT_EQ(ast.nodes[system].payload.flags & AST_FLAG_IS_SYSTEM, AST_FLAG_IS_SYSTEM);
    ASSERT_EQ(ast.nodes[local].payload.flags & AST_FLAG_IS_SYSTEM, 0);

    ast_free(&ast);
}

TEST(test_ast_push_fn_decl)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t consuming = ast_push_fn_decl(&ast, 0, true);
    const uint32_t normal = ast_push_fn_decl(&ast, 1, false);

    assert_node(&ast, consuming, AST_FN_DECL, 0);
    assert_node(&ast, normal, AST_FN_DECL, 1);
    ASSERT_EQ(ast.nodes[consuming].payload.flags & AST_FLAG_CONSUMING, AST_FLAG_CONSUMING);
    ASSERT_EQ(ast.nodes[normal].payload.flags & AST_FLAG_CONSUMING, 0);

    ast_free(&ast);
}

TEST(test_ast_push_struct_decl)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_struct_decl(&ast, 5);

    assert_node(&ast, node_index, AST_STRUCT_DECL, 5);

    ast_free(&ast);
}

TEST(test_ast_push_enum_decl)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_enum_decl(&ast, 5);

    assert_node(&ast, node_index, AST_ENUM_DECL, 5);

    ast_free(&ast);
}

TEST(test_ast_push_trait_decl)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_trait_decl(&ast, 5);

    assert_node(&ast, node_index, AST_TRAIT_DECL, 5);

    ast_free(&ast);
}

TEST(test_ast_push_impl)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t trait_impl = ast_push_impl(&ast, 0, true);
    const uint32_t inherent = ast_push_impl(&ast, 1, false);

    assert_node(&ast, trait_impl, AST_IMPL, 0);
    assert_node(&ast, inherent, AST_IMPL, 1);
    ASSERT_EQ(ast.nodes[trait_impl].payload.flags & AST_FLAG_IS_TRAIT_IMPL, AST_FLAG_IS_TRAIT_IMPL);
    ASSERT_EQ(ast.nodes[inherent].payload.flags & AST_FLAG_IS_TRAIT_IMPL, 0);

    ast_free(&ast);
}

TEST(test_ast_push_const_decl)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_const_decl(&ast, 5);

    assert_node(&ast, node_index, AST_CONST_DECL, 5);

    ast_free(&ast);
}

TEST(test_ast_push_extern_block)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_extern_block(&ast, 5);

    assert_node(&ast, node_index, AST_EXTERN_BLOCK, 5);

    ast_free(&ast);
}

TEST(test_ast_push_type)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_type(&ast, 2, 100);

    assert_node(&ast, node_index, AST_TYPE, 2);
    ASSERT_EQ(ast.nodes[node_index].payload.symbol_id, 100);

    ast_free(&ast);
}

TEST(test_ast_push_ptr_type)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_ptr_type(&ast, 0);

    assert_node(&ast, node_index, AST_PTR_TYPE, 0);

    ast_free(&ast);
}

TEST(test_ast_push_heap_type)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_heap_type(&ast, 0);

    assert_node(&ast, node_index, AST_HEAP_TYPE, 0);

    ast_free(&ast);
}

TEST(test_ast_push_ref_type)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t mut = ast_push_ref_type(&ast, 0, true);
    const uint32_t readonly = ast_push_ref_type(&ast, 1, false);

    assert_node(&ast, mut, AST_REF_TYPE, 0);
    assert_node(&ast, readonly, AST_REF_TYPE, 1);
    ASSERT_EQ(ast.nodes[mut].payload.flags & AST_FLAG_IS_MUT, AST_FLAG_IS_MUT);
    ASSERT_EQ(ast.nodes[readonly].payload.flags & AST_FLAG_IS_MUT, 0);

    ast_free(&ast);
}

TEST(test_ast_push_generic_args)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_generic_args(&ast, 3);

    assert_node(&ast, node_index, AST_GENERIC_ARGS, 3);

    ast_free(&ast);
}

TEST(test_ast_push_block)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_block(&ast, 4);

    assert_node(&ast, node_index, AST_BLOCK, 4);

    ast_free(&ast);
}

TEST(test_ast_push_var_decl)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t typed = ast_push_var_decl(&ast, 0, true);
    const uint32_t inferred = ast_push_var_decl(&ast, 1, false);

    assert_node(&ast, typed, AST_VAR_DECL, 0);
    assert_node(&ast, inferred, AST_VAR_DECL, 1);
    ASSERT_EQ(ast.nodes[typed].payload.flags & AST_FLAG_HAS_EXPLICIT_TYPE, AST_FLAG_HAS_EXPLICIT_TYPE);
    ASSERT_EQ(ast.nodes[inferred].payload.flags & AST_FLAG_HAS_EXPLICIT_TYPE, 0);

    ast_free(&ast);
}

TEST(test_ast_push_assign)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_assign(&ast, 3, TOKEN_PLUS_ASSIGN);

    assert_node(&ast, node_index, AST_ASSIGN, 3);
    ASSERT_EQ(ast.nodes[node_index].payload.op, TOKEN_PLUS_ASSIGN);

    ast_free(&ast);
}

TEST(test_ast_push_return)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_return(&ast, 3);

    assert_node(&ast, node_index, AST_RETURN, 3);

    ast_free(&ast);
}

TEST(test_ast_push_if)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_if(&ast, 3);

    assert_node(&ast, node_index, AST_IF, 3);

    ast_free(&ast);
}

TEST(test_ast_push_while)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_while(&ast, 3);

    assert_node(&ast, node_index, AST_WHILE, 3);

    ast_free(&ast);
}

TEST(test_ast_push_for)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_for(&ast, 3);

    assert_node(&ast, node_index, AST_FOR, 3);

    ast_free(&ast);
}

TEST(test_ast_push_break)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_break(&ast, 3);

    assert_node(&ast, node_index, AST_BREAK, 3);

    ast_free(&ast);
}

TEST(test_ast_push_continue)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_continue(&ast, 3);

    assert_node(&ast, node_index, AST_CONTINUE, 3);

    ast_free(&ast);
}

TEST(test_ast_push_ident)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_ident(&ast, 2, 55);

    assert_node(&ast, node_index, AST_IDENT, 2);
    ASSERT_EQ(ast.nodes[node_index].payload.symbol_id, 55);

    ast_free(&ast);
}

TEST(test_ast_push_int_literal)
{
    Ast ast;
    ast_init(&ast, "42");
    Tokenizer tokenizer = tokenizer_new("42");
    const Token token = get_next_token(&tokenizer);
    const uint32_t token_index = ast_push_token(&ast, token);

    const uint32_t node_index = ast_push_int_literal(&ast, token_index, 42);

    assert_node(&ast, node_index, AST_INT_LITERAL, token_index);
    ASSERT_EQ(ast.nodes[node_index].payload.int_value, 42);

    ast_free(&ast);
}

TEST(test_ast_push_float_literal)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_float_literal(&ast, 2, 0x400921fb54442d18ULL);

    assert_node(&ast, node_index, AST_FLOAT_LITERAL, 2);
    ASSERT_EQ(ast.nodes[node_index].payload.float_bits, 0x400921fb54442d18ULL);

    ast_free(&ast);
}

TEST(test_ast_push_string_literal)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_string_literal(&ast, 2, 77);

    assert_node(&ast, node_index, AST_STRING_LITERAL, 2);
    ASSERT_EQ(ast.nodes[node_index].payload.symbol_id, 77);

    ast_free(&ast);
}

TEST(test_ast_push_char_literal)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_char_literal(&ast, 2, 97);

    assert_node(&ast, node_index, AST_CHAR_LITERAL, 2);
    ASSERT_EQ(ast.nodes[node_index].payload.char_value, 97);

    ast_free(&ast);
}

TEST(test_ast_push_call)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_call(&ast, 3);

    assert_node(&ast, node_index, AST_CALL, 3);

    ast_free(&ast);
}

TEST(test_ast_push_method_call)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_method_call(&ast, 2, 88);

    assert_node(&ast, node_index, AST_METHOD_CALL, 2);
    ASSERT_EQ(ast.nodes[node_index].payload.symbol_id, 88);

    ast_free(&ast);
}

TEST(test_ast_push_struct_literal)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_struct_literal(&ast, 3);

    assert_node(&ast, node_index, AST_STRUCT_LITERAL, 3);

    ast_free(&ast);
}

TEST(test_ast_push_binary_op)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_binary_op(&ast, 3, TOKEN_PLUS);

    assert_node(&ast, node_index, AST_BINARY_OP, 3);
    ASSERT_EQ(ast.nodes[node_index].payload.op, TOKEN_PLUS);

    ast_free(&ast);
}

TEST(test_ast_push_unary_op)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_unary_op(&ast, 3, TOKEN_MINUS);

    assert_node(&ast, node_index, AST_UNARY_OP, 3);
    ASSERT_EQ(ast.nodes[node_index].payload.op, TOKEN_MINUS);

    ast_free(&ast);
}

TEST(test_ast_push_index)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_index(&ast, 3);

    assert_node(&ast, node_index, AST_INDEX, 3);

    ast_free(&ast);
}

TEST(test_ast_push_field)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_field(&ast, 2, 66);

    assert_node(&ast, node_index, AST_FIELD, 2);
    ASSERT_EQ(ast.nodes[node_index].payload.symbol_id, 66);

    ast_free(&ast);
}

TEST(test_ast_indices_survive_relocation)
{
    Ast ast;
    ast_init(&ast, "");

    const uint32_t node_index = ast_push_int_literal(&ast, 0, 42);
    for (uint32_t i = 1; i < 1000; i++)
    {
        ast_push_int_literal(&ast, i, i);
    }

    // Growing the array may move it; indices must remain valid.
    ASSERT_EQ(ast.nodes[node_index].payload.int_value, 42);
    ASSERT_EQ(ast.nodes[500].payload.int_value, 500);

    ast_free(&ast);
}

RUN_ALL_TESTS()