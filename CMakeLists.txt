cmake_minimum_required(VERSION 4.3)
project(Wev C)

set(CMAKE_C_STANDARD 11)

enable_testing()

# Core library shared between the executable and the tests.
add_library(wev_core STATIC
        src/ast/ast.c
        src/ast/ast.h
        src/core/string.c
        src/core/string_builder.c
        src/core/file.c
        src/driver/driver.c
        src/driver/console.c
        src/driver/driver_args.c
        src/parser/parse_decl.c
        src/parser/parse_expr.c
        src/parser/parse_stmt.c
        src/parser/parse_type.c
        src/parser/parser.c
        src/parser/parser.h
        src/parser/parser_internal.h
        src/version.h
        src/token/token.h
        src/token/token_info.c
        src/token/token_info.h
        src/token/tokenizer.c
        src/token/tokenizer.h)

target_include_directories(wev_core PUBLIC src)

add_executable(wev src/main.c)
target_link_libraries(wev PRIVATE wev_core)

# Unit tests
function(wev_add_unit_test target_name path)
    add_executable(${target_name} tests/${path}.c)
    target_link_libraries(${target_name} PRIVATE wev_core)
    target_include_directories(${target_name} PRIVATE tests)
    add_test(NAME ${target_name} COMMAND ${target_name})
endfunction()

wev_add_unit_test(test_string core/test_string)
wev_add_unit_test(test_ast ast/test_ast)
wev_add_unit_test(test_parser parser/test_parser)
wev_add_unit_test(test_file core/test_file)
wev_add_unit_test(test_string_builder core/test_string_builder)
wev_add_unit_test(test_console driver/test_console)
wev_add_unit_test(test_tokenizer token/test_tokenizer)
wev_add_unit_test(test_token_info token/test_token_info)
wev_add_unit_test(test_driver_args driver/test_driver_args)
wev_add_unit_test(test_driver driver/test_driver)
