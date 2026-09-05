# Wev AST Design

Status: working design for MVP1

This document describes the AST representation of the Wev compiler and the
catalogue of node types with their implementation status. It was extracted from
`wev_design.md` so that the node catalogue can live next to its implementation
state and be updated type by type.

## 1. Representation

The AST is stored as a **flat array of nodes referenced by indices**, not as
individually allocated nodes with pointers. This section describes the
representation and why it is preferred.

### 1.1 Shape

An `Ast` owns three contiguous arrays plus the arena for auxiliary data:

```c
typedef struct AstNode
{
    uint32_t kind;         // AstNodeKind
    uint32_t token_index;  // index into Ast.tokens
    uint32_t first_child;  // index into Ast.nodes (start of child range)
    uint32_t child_count;  // number of children in the range
    union
    {
        uint64_t int_value;   // AST_INT_LITERAL
        uint64_t float_bits;  // AST_FLOAT_LITERAL
        uint32_t symbol_id;   // AST_IDENT, AST_TYPE, strings, method/field names
        uint32_t char_value;  // AST_CHAR_LITERAL
        TokenType op;         // AST_ASSIGN, AST_BINARY_OP, AST_UNARY_OP
        uint32_t flags;       // AST_FLAG_* boolean modifiers
    } payload;
} AstNode;

typedef struct Ast
{
    AstNode* nodes;   size_t nodes_count, nodes_capacity;
    Token*   tokens;  size_t tokens_count, tokens_capacity;
    const char* source;   // kept alive for offset-based token reads
    Arena   arena;        // interned symbols, de-escaped strings
} Ast;
```

Children are stored as **contiguous ranges**: `first_child` plus `child_count`.
There are no `AstNode*` pointers anywhere in the structure; all cross-references
are `uint32_t` indices into one of the arrays.

### 1.2 Building

A recursive-descent parser appends nodes to the array. Lists are built by
recording the start index before parsing the items, then writing the range:

```c
uint32_t start = ast->nodes_count;
// ... parse each child with ast_push(...)
uint32_t node = ast_push(ast, AST_BLOCK, token_index);
ast->nodes[node].first_child = start;
ast->nodes[node].child_count = node - start;
```

Node insertion never moves previously emitted nodes: `ast_push` only appends,
and a single `realloc` may move the whole array, which is safe because
references are indices, not addresses.

### 1.3 Traversal

Reading a node and iterating its children is a few dependent loads:

```c
const AstNode* n = &ast->nodes[idx];
for (uint32_t i = n->first_child; i < n->first_child + n->child_count; i++)
{
    visit(ast, i);
}
```

Tokens are read by index the same way. Because `Token` stores offsets
(`location`, `length`) instead of a `str_ptr`, the text of any token is
re-derived as `source + location`, so the token array contains no pointers.

### 1.4 Why indices over pointers

- **Locality**: nodes are packed contiguously and visited in index order, so a
  traversal touches few cache lines. Boxed, separately allocated nodes scatter
  children across the heap and force pointer chasing on every step.
- **Compactness**: a node is roughly 16-24 bytes with `uint32_t` fields and a
  small payload union; a pointer-based design spends 8 bytes per child and
  varies node size, causing internal fragmentation.
- **No per-node allocation**: the parser only ever appends to arrays; there is
  no `malloc` per node and exactly one `free` at the end of the unit.
- **Realloc-safe**: growing the node array may move it, but indices remain
  valid. Pointer-based arenas cannot move without rewriting every reference.
- **Relocatable and serializable**: a structure made only of indices and
  offsets can be written to disk and reloaded verbatim, with no pointer
  fix-ups. This is what makes AST and interface caching cheap
  (`wev_design.md` Section 22).
- **Side tables for analysis**: later passes (type checking, ownership, drop
  insertion) can attach per-node data in parallel arrays indexed by node id,
  leaving the AST itself read-only. The same is awkward with boxed nodes.

### 1.5 Symbols and strings

Identifiers and type names are interned once into the `Arena`; the node stores a
`symbol_id` instead of a source range. Pointer-equality of interned strings
becomes integer equality of ids, which speeds up name resolution. String
literals keep their token offset and are de-escaped into the arena only when
the C backend needs the concrete bytes.

### 1.6 The token array

The lexer already yields one `Token` at a time; the parser driver collects them
into `Ast.tokens` while tokenizing the whole source up front. The AST then
references tokens by index. Keeping the tokens in the same `Ast` means the AST
owns everything it references, and the whole unit is freed together.

## 2. Node catalogue

One line per `AstNodeKind`. `Children` lists the ordered child semantics,
`Payload` names the union field used (if any), `Token` is the primary token
referenced by the node. `Status` tracks implementation: `done` once the node
type has a builder function in `src/ast/ast.c` and unit tests in
`tests/ast/test_ast.c`. The parser itself is built separately on top of these
builders.

The payload stays a single 8-byte union; `symbol_id` values point into the
interned-symbol arena (1.5). Boolean modifiers (`is_system`, `consuming`,
`is_trait_impl`, `has_explicit_type`, `is_mut`) are packed into a
single `flags` field with `AST_FLAG_*` masks; operators (`AST_ASSIGN`,
`AST_BINARY_OP`, `AST_UNARY_OP`) store a `TokenType` in `op`. `AST_VAR_DECL`
is reused for function parameters, struct fields, and enum variants. A heap
construction `^User { ... }` is an `AST_HEAP_TYPE` wrapping an
`AST_STRUCT_LITERAL`.

| Kind | Children | Payload | Token | Status |
| --- | --- | --- | --- | --- |
| `AST_MODULE` | declarations top-level | — | first token | done |
| `AST_INCLUDE` | — | `is_system` | header name | done |
| `AST_FN_DECL` | parameters…, return type? | `consuming` | function name | done |
| `AST_STRUCT_DECL` | fields… | — | struct name | done |
| `AST_ENUM_DECL` | variants… | — | enum name | done |
| `AST_TRAIT_DECL` | methods… | — | trait name | done |
| `AST_IMPL` | target type, members… | `is_trait_impl` | target name | done |
| `AST_CONST_DECL` | type?, initializer | — | const name | done |
| `AST_EXTERN_BLOCK` | extern declarations… | — | `extern` | done |
| `AST_TYPE` | generic args… | `symbol_id` | type name | done |
| `AST_HEAP_TYPE` | pointee type | — | `^` | done |
| `AST_PTR_TYPE` | pointee type | — | `*` | done |
| `AST_REF_TYPE` | referent type | `is_mut` | `&` | done |
| `AST_GENERIC_ARGS` | types… | — | `<` | done |
| `AST_BLOCK` | statements… | — | `{` | done |
| `AST_VAR_DECL` | type?, initializer? | `has_explicit_type` | variable name | done |
| `AST_ASSIGN` | lhs, rhs | `op` | operator | done |
| `AST_RETURN` | value? | — | `return` | done |
| `AST_IF` | condition, then, else? | — | `if` | done |
| `AST_WHILE` | condition, body | — | `while` | done |
| `AST_FOR` | iterator, collection, body | — | `for` | done |
| `AST_BREAK` | — | — | `break` | done |
| `AST_CONTINUE` | — | — | `continue` | done |
| `AST_IDENT` | — | `symbol_id` | name | done |
| `AST_INT_LITERAL` | — | `int_value` | literal | done |
| `AST_FLOAT_LITERAL` | — | `float_bits` | literal | done |
| `AST_STRING_LITERAL` | — | `symbol_id` (de-escaped) | literal | done |
| `AST_CHAR_LITERAL` | — | `char_value` | literal | done |
| `AST_CALL` | callee, arguments… | — | `(` | done |
| `AST_METHOD_CALL` | receiver, arguments… | `method_symbol_id` | method name | done |
| `AST_STRUCT_LITERAL` | type, field name/value pairs… | — | type name | done |
| `AST_BINARY_OP` | lhs, rhs | `op` | operator | done |
| `AST_UNARY_OP` | operand | `op` (incl. `&`, `&mut`, deref) | operator | done |
| `AST_INDEX` | base, index | — | `[` | done |
| `AST_FIELD` | base | `field_symbol_id` | field name | done |