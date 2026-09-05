#include <string.h>

#include "parser/parser.h"
#include "test_util.h"

static Ast parse_success(const char* source)
{
    Ast ast;
    ast_init(&ast, "");
    ParseError error = {0};
    ASSERT_TRUE(wev_parse_source(source, &ast, &error));
    return ast;
}

static void assert_node(const Ast* ast, const uint32_t index, const AstNodeKind kind, const uint32_t token_index, const uint32_t child_count)
{
    ASSERT_EQ(ast->nodes[index].kind, kind);
    ASSERT_EQ(ast->nodes[index].token_index, token_index);
    ASSERT_EQ(ast->nodes[index].child_count, child_count);
}

static void assert_symbol(const Ast* ast, const uint32_t symbol_id, const char* expected)
{
    ASSERT_EQ(ast->symbols[symbol_id].length, (uint32_t)strlen(expected));
    ASSERT_TRUE(memcmp(ast->source + ast->symbols[symbol_id].offset, expected, ast->symbols[symbol_id].length) == 0);
}

TEST(test_parser_empty_source)
{
    Ast ast = parse_success("");

    ASSERT_EQ(ast.nodes_count, 1);
    ASSERT_EQ(ast.symbols_count, 0);
    assert_node(&ast, 0, AST_MODULE, 0, 0);

    ast_free(&ast);
}

TEST(test_parser_hello_world)
{
    Ast ast = parse_success("fn main() {\n    println(\"Hello from Wev\")\n}\n");

    ASSERT_EQ(ast.nodes_count, 6);

    assert_node(&ast, 0, AST_IDENT, 5, 0);
    assert_symbol(&ast, ast.nodes[0].payload.symbol_id, "println");

    assert_node(&ast, 1, AST_STRING_LITERAL, 7, 0);
    assert_symbol(&ast, ast.nodes[1].payload.symbol_id, "Hello from Wev");

    assert_node(&ast, 2, AST_CALL, 6, 2);
    ASSERT_EQ(ast.nodes[2].first_child, 0);

    assert_node(&ast, 3, AST_BLOCK, 4, 1);
    ASSERT_EQ(ast.nodes[3].first_child, 2);

    assert_node(&ast, 4, AST_FN_DECL, 1, 1);
    ASSERT_EQ(ast.nodes[4].first_child, 3);

    assert_node(&ast, 5, AST_MODULE, 0, 1);
    ASSERT_EQ(ast.nodes[5].first_child, 4);

    ast_free(&ast);
}

TEST(test_parser_call_arguments)
{
    Ast ast = parse_success("fn main() { f(1, 2.5, \"x\") }");

    assert_node(&ast, 0, AST_IDENT, 5, 0);
    assert_node(&ast, 1, AST_INT_LITERAL, 7, 0);
    ASSERT_EQ(ast.nodes[1].payload.int_value, 1);

    assert_node(&ast, 2, AST_FLOAT_LITERAL, 9, 0);
    double float_value;
    memcpy(&float_value, &ast.nodes[2].payload.float_bits, sizeof(float_value));
    ASSERT_TRUE(float_value == 2.5);

    assert_node(&ast, 3, AST_STRING_LITERAL, 11, 0);
    assert_symbol(&ast, ast.nodes[3].payload.symbol_id, "x");

    assert_node(&ast, 4, AST_CALL, 6, 4);
    ASSERT_EQ(ast.nodes[4].first_child, 0);

    ast_free(&ast);
}

TEST(test_parser_int_literal_hex)
{
    Ast ast = parse_success("fn main() { f(0xFF) }");

    assert_node(&ast, 1, AST_INT_LITERAL, 7, 0);
    ASSERT_EQ(ast.nodes[1].payload.int_value, 255);

    ast_free(&ast);
}

TEST(test_parser_symbols_deduplicated)
{
    Ast ast = parse_success("fn main() { println(main, main) }");

    ASSERT_EQ(ast.symbols_count, 2);
    assert_symbol(&ast, ast.nodes[0].payload.symbol_id, "println");
    ASSERT_EQ(ast.nodes[1].payload.symbol_id, ast.nodes[2].payload.symbol_id);
    assert_symbol(&ast, ast.nodes[1].payload.symbol_id, "main");

    ast_free(&ast);
}

TEST(test_parser_return_type)
{
    Ast ast = parse_success("fn main() -> i32 { 42 }");

    assert_node(&ast, 0, AST_TYPE, 5, 0);
    assert_symbol(&ast, ast.nodes[0].payload.symbol_id, "i32");

    assert_node(&ast, 1, AST_INT_LITERAL, 7, 0);
    assert_node(&ast, 2, AST_BLOCK, 6, 1);
    assert_node(&ast, 3, AST_FN_DECL, 1, 3);
    assert_node(&ast, 4, AST_MODULE, 0, 1);

    ast_free(&ast);
}

TEST(test_parser_error_missing_brace)
{
    Ast ast;
    ast_init(&ast, "");
    ParseError error = {0};

    ASSERT_TRUE(!wev_parse_source("fn main() { println(\"hi\")", &ast, &error));
    ASSERT_TRUE(error.message != NULL);
    ASSERT_EQ(error.line, 1);
    ASSERT_TRUE(error.col > 0);

    ast_free(&ast);
}

TEST(test_parser_error_unterminated_call)
{
    Ast ast;
    ast_init(&ast, "");
    ParseError error = {0};

    ASSERT_TRUE(!wev_parse_source("fn main() { println(\"hi\" }", &ast, &error));
    ASSERT_TRUE(error.message != NULL);

    ast_free(&ast);
}

TEST(test_parser_error_unknown_top_level)
{
    Ast ast;
    ast_init(&ast, "");
    ParseError error = {0};

    ASSERT_TRUE(!wev_parse_source("x := 1", &ast, &error));
    ASSERT_TRUE(error.message != NULL);

    ast_free(&ast);
}

TEST(test_parser_error_missing_fn_name)
{
    Ast ast;
    ast_init(&ast, "");
    ParseError error = {0};

    ASSERT_TRUE(!wev_parse_source("fn () { }", &ast, &error));
    ASSERT_TRUE(error.message != NULL);

    ast_free(&ast);
}

RUN_ALL_TESTS()