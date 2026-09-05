# AGENTS.md

## Project

Wev is a small, modern systems language that transpiles to portable C. It aims
to prevent common ownership and dangling-reference mistakes without reproducing
the full Rust type system. The compiler is currently at a very early stage
(MVP1).

## Design reference

The language design is specified in `docs/wev_design.md` (status: working design
for MVP1). Read it before working on the compiler: it covers the type model
(ownership, `*T`, `&T`), syntax, and the compiler pipeline. The AST
representation and node catalogue are specified in `docs/ast_design.md`.
Current compiler state corresponds to the start of that pipeline
(lexer/tokenizer only).

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
src/ast/              AST container and builder (nodes, tokens, source)
src/core/             String, string builder, and file helpers
src/driver/           Driver (CLI entry), console, argument parsing
src/token/            Tokenizer and token info (current compiler stage)
src/version.h         Version macro (WEV_VERSION "0.1.0")
tests/                Unit tests (one executable per test file, run via CTest)
tests_src/            Wev sample sources (e.g. hello_world.w)
docs/                 Language and AST design documents (wev_design.md, ast_design.md)
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

Note: prefer the CMake bundled with CLion when it is present, falling back to
the one on PATH otherwise:

```sh
CLION_CMAKE="$HOME/.local/share/clion/bin/cmake/linux/x64/bin/cmake"
if test -x "$CLION_CMAKE"; then
    cmake() { "$CLION_CMAKE" "$@"; }
fi
```

## Code quality (match CLion)

Generated code must satisfy the same static checks CLion reports. Verify after
every change.

### Compiler warnings

Compile with the same warning flags CLion surfaces in the editor, in addition
to what CMakeLists.txt already sets:

```sh
CLANG="${CLANG:-clang}"
for f in $(find src tests -name '*.c'); do
    "$CLANG" -std=gnu11 -I src -I tests \
        -Wall -Wextra -Wshadow -Wconversion -fsyntax-only "$f"
done
```

Treat any diagnostic as a failure: fix it (or explicitly justify it) before
moving on. `-Wshadow` in particular flags local declarations that hide an
outer one, e.g. redeclaring `token` inside a function that already has one.

### clang-tidy

The project pins its clang-tidy configuration in `.clang-tidy` (same profile as
the `zenc` project). CLion reads this file automatically, and so must generated
code. Run the checks with the project's compile database:

```sh
CLANG_TIDY="${CLANG_TIDY:-$HOME/.local/share/clion/bin/clang/linux/x64/bin/clang-tidy}"
for f in $(find src -name '*.c'); do
    "$CLANG_TIDY" -p cmake-build-debug "$f"
done
```

Note: the CLion clang-tidy (LLVM) does not resolve the GCC toolchain's system
include paths on its own, so `clang-diagnostic-error` for headers such as
`stddef.h`, `stdbool.h` or `stdarg.h` is a false positive. When that happens,
add the GCC system include directories explicitly with `--extra-arg=-isystem`
(adjust the GCC version directory if needed):

```sh
GCC_INC="/usr/lib/gcc/x86_64-linux-gnu/13/include"
CLANG_TIDY="${CLANG_TIDY:-$HOME/.local/share/clion/bin/clang/linux/x64/bin/clang-tidy}"
for f in $(find src -name '*.c'); do
    "$CLANG_TIDY" -p cmake-build-debug "$f" \
        --extra-arg=-isystem --extra-arg="$GCC_INC" \
        --extra-arg=-isystem --extra-arg=/usr/include/x86_64-linux-gnu \
        --extra-arg=-isystem --extra-arg=/usr/include
done
```

Resolve every warning or error the checks report before finishing the change.

### Formatting

clang-format must be applied to all touched files (see `.clang-format`):

```sh
clang-format -i <files>
```