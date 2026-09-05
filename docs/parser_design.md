# Wev Parser Design

Status: working design for MVP1

This document describes the architecture of the Wev parser and, most
importantly, the **MVP1 parsing checklist**: every construct the parser must
accept for the language defined in `wev_design.md` to be complete. It is the
work plan for the parser module (`src/parser/`). The AST representation and the
node builders are in `ast_design.md`; this document assumes them.

## 1. Role and scope

The parser sits between the tokenizer (`src/token/`) and the semantic passes
(`src/sema/`, pipeline `wev_design.md` §23):

```text
source -> lexer -> parser -> module resolution -> name resolution -> ...
```

The parser produces an `Ast` (flat node and token arrays) over the whole
source. It is a recursive-descent parser; children are stored as contiguous
index ranges (`ast_begin_children` / `ast_end_children`).

The parser is a library. It is not wired into the `wev` CLI driver yet; that
comes once the module is stable.

## 2. Architecture

```text
src/parser/
    parser.h           // public API: ParseError, wev_parse_source
    parser_internal.h  // Parser state + helpers shared by the parse_*.c files
    parser.c           // entry, token driver, top-level (AST_MODULE), errors
    parse_decl.c       // top-level declarations (fn, struct, enum, trait, impl, const, extern, include)
    parse_stmt.c       // statements (blocks, var decl, assign, control flow)
    parse_expr.c       // expressions (literals, calls, postfix, precedence)
    parse_type.c       // types (primitives, named, ^T, &T, &mut T, *T, T<...>)
```

### 2.1 Parser state

```c
typedef struct Parser
{
    Ast* ast;            // nodes, tokens, and interned symbols being produced
    const char* source;  // kept alive to read token text
    size_t cursor;       // position in ast->tokens
    ParseError error;    // first error encountered
    bool failed;
} Parser;
```

### 2.2 Public API

```c
bool wev_parse_source(const char* source, Ast* ast, ParseError* out_error);
```

Flow: tokenize the whole source into `ast->tokens` (`ast_design.md` §1.6), then
parse the top-level declarations under a root `AST_MODULE`. `cursor` walks
`ast->tokens`; every node references its primary `token_index` and its children
as a range.

### 2.3 Parsing strategy

- **Statement boundaries** — the tokenizer discards newlines, so statements are
  delimited by grammar only: a statement ends when the next token cannot
  continue the current expression, or on `}` / EOF. A `;` is accepted as an
  explicit separator. Known edge case: `x := 1` followed by `-y` on the next
  line is read as `x := 1 - y`.
- **Symbol interning** — symbols live on the `Ast` (`ast_intern`, `Ast.symbols`,
  a linear-scan table referencing `{offset, length}` into the source; the
  project has no hash table). `symbol_id` values feed the `AST_IDENT`,
  `AST_TYPE`, and `AST_STRING_LITERAL` payloads. String literals are interned
  without their surrounding quotes (`ast_design.md` §1.5).
- **Child ranges** — the parser pushes children, then the parent node, and
  records `first_child` = index of the node's first *direct* child with
  `child_count = node_index - first_child` (`ast_end_children`). For the M1
  constructs (one nested child per container, leaf children in lists) this is
  exactly the direct children. When a parent has several children whose own
  children are pushed inline, the range spans the whole construct (interleaved
  grandchildren included); exact direct-children navigation will be added with
  the M2 constructs that need it.
- **Literals** — ints (dec/hex) parsed manually into `uint64_t`, floats via
  `strtod` into the `float_bits` payload; strings are interned raw (without
  quotes) and de-escaped later by the backend (`ast_design.md` §1.5).
- **Errors** — first error wins: `ParseError { line, col, message }`, filled
  from the current token's position; `wev_parse_source` returns false.
- **Precedence** — recursive-descent, one function per level (no Pratt/Yacc
  table). Binary levels, tightest to loosest: `* / %`, `+ -`, `< > <= >=`,
  `== !=`, `&&`, `||`. Prefix unary (`- ! ~ & &mut *`) binds tighter than any
  binary operator; postfix (`() [] .`) binds tighter still. `&` and `*` are
  disambiguated by calling `parse_type` vs `parse_expr`; `^` is not ambiguous
  (reserved for the heap type, `wev_design.md` §26.11).

  Each binary level loops `while (match(op))`, giving left associativity
  naturally (`a-b-c` = `(a-b)-c`). Because the AST is a flat index array, the
  `AST_BINARY_OP` node is created *after* both operands are in the array, so
  the child range is known without moving any node (`ast_design.md` §1.2):

  ```c
  static uint32_t make_binary(Parser* p, const uint32_t lhs, const uint32_t rhs)
  {
      const uint32_t node = ast_push_binary_op(p->ast, prev_token_index(p), prev_token_type(p));
      p->ast->nodes[node].first_child = lhs;          // lhs == the recorded start
      p->ast->nodes[node].child_count = rhs - lhs + 1;
      return node;
  }
  ```

## 3. MVP1 parsing checklist

Status legend:

- `todo` — to implement in `src/parser/`.
- `blocked` — needs a tokenizer or AST addition first (gap in §4).

### 3.1 Infrastructure

| # | Construct | Notes | Status |
| --- | --- | --- | --- |
| 1 | Token driver: tokenize whole source into `ast->tokens` | `ast_design.md` §1.6 | todo |
| 2 | Top-level driver under `AST_MODULE` | empty source -> empty module | todo |
| 3 | Symbol interner (`symbol_id`) | | todo |
| 4 | Error reporting `ParseError` | line/col/message, first-error-wins | todo |
| 5 | Statement boundary (grammar-only) + optional `;` | §2.3 | todo |
| 6 | Literal conversion (int/float/string/char) | | todo |

### 3.2 Top-level declarations

| # | Construct | Example | AST | Status |
| --- | --- | --- | --- | --- |
| 7 | include | `include <stdio.h>` / `include "h.h"` | AST_INCLUDE (is_system) | todo |
| 8 | function | `fn main()` | AST_FN_DECL | todo |
| 9 | consuming method | `consuming fn into_name()` | AST_FN_DECL (consuming) | todo |
| 10 | parameters | `fn f(x: i32, y: &User)` | AST_VAR_DECL children | todo |
| 11 | return type | `fn f() -> &String` | child of AST_FN_DECL | todo |
| 12 | struct | `struct User { ... }` | AST_STRUCT_DECL | todo |
| 13 | struct fields | `name: String` | AST_VAR_DECL children | todo |
| 14 | enum | `enum Color { Red Green }` | AST_ENUM_DECL | todo |
| 15 | enum variants | `Ok(T)` / `None` | AST_VAR_DECL children | todo |
| 16 | trait | `trait Printable { fn print() }` | AST_TRAIT_DECL | todo |
| 17 | inherent impl | `impl User { ... }` | AST_IMPL | todo |
| 18 | trait impl | `impl Printable for User` | AST_IMPL (is_trait_impl) | todo |
| 19 | const | `const MaxUsers: usize = 100` | AST_CONST_DECL | todo |
| 20 | extern block | `extern "c" { ... }` | AST_EXTERN_BLOCK | todo |
| 21 | extern fn body | variadic `...`, `*const c_char` | — | blocked |
| 22 | import | `import math` / `import utils.io` / `import foo.math as m` | AST_IMPORT | todo |
| 23 | pub visibility | `pub fn` | — | blocked |

### 3.3 Types

| # | Construct | Example | AST | Status |
| --- | --- | --- | --- | --- |
| 24 | primitives | `bool`, `i32`, `u8`, `isize`, `f64`, `char`, `void` | AST_TYPE | todo |
| 25 | named type | `User`, `String` | AST_TYPE | todo |
| 26 | owned heap | `^T` | AST_HEAP_TYPE | todo |
| 27 | readonly borrow | `&T` | AST_REF_TYPE | todo |
| 28 | mutable borrow | `&mut T` | AST_REF_TYPE (is_mut) | todo |
| 29 | raw pointer | `*T` | AST_PTR_TYPE | todo |
| 30 | raw const/mut pointer | `*const c_char`, `*mut T` (FFI) | — | blocked |
| 31 | generic args | `List<User>`, `Result<Config, IoError>` | AST_GENERIC_ARGS | todo |
| 32 | generic decl params | `enum Result<T, E>` | AST_GENERIC_ARGS on decls | todo |

### 3.4 Statements

| # | Construct | Example | AST | Status |
| --- | --- | --- | --- | --- |
| 33 | block | `{ ... }` | AST_BLOCK | todo |
| 34 | inferred var decl | `x := 42` | AST_VAR_DECL | todo |
| 35 | typed var decl | `count: usize = 10` | AST_VAR_DECL (has_explicit_type) | todo |
| 36 | assignment | `=`, `+=`, `-=`, `*=`, `/=`, `%=` | AST_ASSIGN (op) | todo |
| 37 | return | `return value` | AST_RETURN | todo |
| 38 | if / else | `if c { } else { }` | AST_IF (else branch) | todo |
| 39 | while | `while c { }` | AST_WHILE | todo |
| 40 | for / in | `for item in items { }` | AST_FOR | todo |
| 41 | break / continue | `break`, `continue` | AST_BREAK, AST_CONTINUE | todo |
| 42 | expression statement | `println("hi")` | (no node) | todo |
| 43 | unsafe block | `unsafe { ... }` | — | blocked |
| 44 | error propagation | `load(path)?` | — | blocked |

### 3.5 Expressions

| # | Construct | Example | AST | Status |
| --- | --- | --- | --- | --- |
| 45 | int literal | `42`, `0xFF` | AST_INT_LITERAL | todo |
| 46 | float literal | `3.14`, `1.5e-3` | AST_FLOAT_LITERAL | todo |
| 47 | string literal | `"hello"` (escapes) | AST_STRING_LITERAL | todo |
| 48 | char literal | `'a'` | AST_CHAR_LITERAL | todo |
| 49 | identifier | `user` | AST_IDENT | todo |
| 50 | call | `f(a, b)` | AST_CALL | todo |
| 51 | method call | `user.birthday()` | AST_METHOD_CALL | todo |
| 52 | field access | `user.name` | AST_FIELD | todo |
| 53 | index | `users[0]` | AST_INDEX | todo |
| 54 | struct literal | `User { name: "Alice", age: 30 }` | AST_STRUCT_LITERAL | todo |
| 55 | heap literal | `^User { ... }` | AST_HEAP_TYPE over AST_STRUCT_LITERAL | todo |
| 56 | binary ops | `* / % + - < > <= >= == != && \|\|` | AST_BINARY_OP (op) | todo |
| 57 | unary ops | `- ! ~ & &mut *` | AST_UNARY_OP (op) | todo |
| 58 | postfix chains | `users[0].display()` | composed postfix | todo |

### 3.6 Already covered elsewhere

- `clone()` is an ordinary method call (`AST_METHOD_CALL`) — no special syntax.

## 4. Design gaps and decisions

Items marked `blocked` in §3 need one of these before the parser can handle
them; each is an open point in `wev_design.md`.

| Gap | Impact | Options |
| --- | --- | --- |
| No `pub` token | §21.3, #23 | Add `TOKEN_PUB`; store a visibility flag |
| No `unsafe` AST node | §19, #43 | Add `AST_UNSAFE_BLOCK` (or reuse `AST_BLOCK` + flag) |
| No error-propagation node | §17, #44 | Add a postfix node or fold into `AST_UNARY_OP` |
| Parameter `consuming` not representable | §8, #10 | `AST_FLAG_CONSUMING` on the parameter `AST_VAR_DECL` |
| `*const T` / `*mut T` raw FFI | §18, #30 | Add a raw const/mut flag on `AST_PTR_TYPE` |
| Variadic `...` in extern fns | §18, #21 | Add `TOKEN_ELLIPSIS` |
| `make(...)` vs no `make` | §9, tokenizer tests use `make(...)` | Decide: accept-and-ignore or reject |
| `AST_FN_DECL` body child | body not listed in the `ast_design.md` catalogue | Record: `AST_FN_DECL` children = parameters, return type?, body |
| else-if representation | #38 | else branch holds a nested `AST_IF` |

## 5. Milestones

- **M1 (hello_world.w)** — items 1-5, 8, 11 (empty), 33, 42, 45-47, 49, 50:
  `AST_MODULE -> AST_FN_DECL -> AST_BLOCK -> AST_CALL (AST_IDENT +
  AST_STRING_LITERAL)`.
- **M2** — full type grammar (24-29, 31) + statements (34-41) + remaining
  expressions (48, 51-58).
- **M3** — remaining declarations (7, 12-20).
- **M4** — unblocked gaps from §4.