# AGENTS.md

## Project

Wev is a small, modern systems language that transpiles to portable C. It aims
to prevent common ownership and dangling-reference mistakes without reproducing
the full Rust type system. The compiler is currently at a very early stage
(MVP1).

## Design reference

The language design is specified in `WEV_DESIGN.md` (status: working design for
MVP1). Read it before working on the compiler: it covers the type model
(ownership, `*T`, `&T`), syntax, and the compiler pipeline. Current compiler
state corresponds to the start of that pipeline (lexer/tokenizer only).

## Tech stack

- C11, built with CMake (minimum 4.3).
- Unit tests: plain C executables registered via CTest (`enable_testing()`).
- Code style: clang-format with `.clang-format` (LLVM base, 4-space indent,
  Allman braces, 150-column limit).
- No external library dependencies.

## Project structure

```
CMakeLists.txt        Core library, executable, and unit-test targets
src/main.c            Entrypoint of the driver
src/core/             String, string builder, and file helpers
src/driver/           Driver (CLI entry), console, argument parsing
src/token/            Tokenizer and token info (current compiler stage)
src/version.h         Version macro (WEV_VERSION "0.1.0")
tests/                Unit tests (one executable per test file, run via CTest)
tests_src/            Wev sample sources (e.g. hello_world.w)
```

- `wev_core` is a static library shared by the executable and the tests; it is
  the place to add compiler modules.
- Tests use `tests/test_util.h` helpers. New unit tests are registered in
  `CMakeLists.txt` via `wev_add_unit_test(target tests/path)`.

## Build and test

```sh
cmake -B cmake-build-debug
cmake --build cmake-build-debug
ctest --test-dir cmake-build-debug
```