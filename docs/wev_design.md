# Wev Language Design

Status: working design for MVP1

This document records the current design direction for Wev. It is intentionally
pragmatic: Wev aims to prevent the common ownership and dangling-reference
mistakes without trying to reproduce the complete Rust type system.

The name `Wev` is provisional. The project name, repository name, file
extension, and trademark availability still need to be checked before a public
release.

## 1. Vision

Wev is a small, modern systems language that transpiles to portable C.

Primary goals:

- Keep the language small and easy to learn.
- Make values and ownership understandable from the source code.
- Automatically clean up owned heap allocations.
- Prevent common dangling references and double frees.
- Avoid requiring manual `alloc` and `free` in normal code.
- Interoperate directly with existing C headers, source files, and libraries.
- Produce readable C11 compatible with TinyCC, Clang, and GCC.
- Keep low-level escape hatches available through `unsafe`.

Non-goals for MVP1:

- Full Rust-level memory safety.
- A garbage collector.
- Class inheritance or object-oriented reference semantics.
- C++ compatibility.
- Complete parsing of arbitrary C headers.
- Concurrency safety guarantees.
- A large standard library.

## 2. Design Philosophy

Ownership is the normal rule for every value. `^` only indicates that an owned
value lives on the heap. `&` indicates that a value is not owned.

The core type model is:

```text
T           // owned inline value
^T          // owned heap value

&T          // readonly borrow
&mut T      // mutable borrow

*T          // raw / unmanaged pointer
```

Construction is symmetric:

```text
user := User { ... }     // User
user := ^User { ... }    // ^User
```

Wev uses values by default. Heap allocation is explicit with `^T`, but its
cleanup is automatic.

The language protects the most important ownership invariants:

- An owned allocation has one owner at a time.
- Moving ownership invalidates the source binding.
- A reference does not own or destroy its target.
- A reference cannot outlive its target when the compiler can determine this.
- Duplicating a resource-owning value is explicit (`clone`).

Wev does not try to prevent every aliasing problem. Mutable aliases are
permitted. Wev therefore does not promise the absence of data races or all
logical conflicts caused by shared mutable state.

## 3. Source Files and Commands

Provisional file extension:

```text
.wev
```

Provisional compiler name:

```text
wev
```

Initial commands:

```text
wev build main.wev -o app
wev run main.wev
wev emit-c main.wev -o main.c
wev build main.wev -collection:foo=/path/to/repo
```

External modules are bound at build time with `-collection:name=path`
(Section 21).

The C compiler is selectable:

```text
wev run main.wev --cc tcc
wev build main.wev --cc clang
wev build main.wev --cc gcc
```

## 4. Basic Syntax

Functions use the conventional `fn` keyword:

```text
fn main() {
    println("Hello from Wev")
}
```

Variables use Odin-like declaration syntax:

```text
x := 42
name := "Alice"

count: usize = 10
count = 20
```

Rules:

- `:=` declares and initializes a variable with inferred type.
- `name: Type` declares a variable with an explicit type.
- `=` assigns to an existing variable.
- Variables are mutable by default.
- `const` declares a compile-time or immutable constant, depending on the
  eventual constant-expression rules.

Example:

```text
const MaxUsers: usize = 100
```

Semicolon policy is still open. The preferred direction is optional
semicolons, with newlines and braces determining statement boundaries where
possible.

## 5. Primitive Types

The initial primitive type set is:

```text
bool

i8   i16   i32   i64
u8   u16   u32   u64

isize
usize

f32
f64

char
void
```

Integer widths are guaranteed by the language:

```text
i32 == exactly 32 signed bits
u64 == exactly 64 unsigned bits
```

`usize` is an unsigned pointer-sized integer. `isize` is a signed
pointer-sized integer.

The C mapping is approximately:

```text
i8    -> int8_t
i16   -> int16_t
i32   -> int32_t
i64   -> int64_t
u8    -> uint8_t
u16   -> uint16_t
u32   -> uint32_t
u64   -> uint64_t
usize -> size_t
isize -> ptrdiff_t
f32   -> float
f64   -> double
bool  -> bool
```

Platform-dependent names such as `int`, `long`, and `uint` are not part of the
normal Wev type system. C-specific aliases may exist under a dedicated `c`
namespace for FFI.

`char` is a Unicode scalar value. A C byte is represented by `u8`; a C `char`
uses an explicit FFI type such as `c_char`.

Signed arithmetic overflow must not rely on C undefined behavior. The exact
MVP policy is still open, but the preferred default is checked overflow with a
runtime failure. Wrapping and saturating operations may be added explicitly
later.

## 6. Structs

Wev has structs, not classes:

```text
struct User {
    name: String
    age: i32
}
```

Structs have value semantics. They do not implicitly carry reference identity
or inheritance.

A struct can contain:

- primitive values;
- other structs;
- enums;
- owned heap values (`^T`);
- collections;
- non-owning references, subject to lifetime checks;
- raw pointers only in `unsafe` code.

## 7. Methods

Methods are defined in an `impl` block. The receiver is implicit because the
enclosing type is known:

```text
impl User {
    fn birthday() {
        age += 1
    }

    fn name() -> &String {
        return name
    }
}
```

Usage:

```text
user.birthday()
name := user.name()
```

Inside an instance method, fields can be accessed directly. `self` is not
required in the source syntax.

Normal methods borrow their receiver and do not take ownership. Read-only
methods operate on a `&` borrow; methods that mutate the receiver operate on a
`&mut` borrow. Consistent with Wev's permissive aliasing model, `&mut` is not
an exclusive borrow.

## 8. Consuming Methods and Parameters

The `consuming` modifier explicitly transfers ownership of a receiver or
parameter.

```text
impl User {
    consuming fn into_name() -> String {
        return name
    }
}
```

After calling the method, the original binding is no longer usable:

```text
name := user.into_name()
user.birthday() // error: user was consumed
```

Normal functions can consume parameters as well:

```text
fn store_user(user: consuming User) {
    // this function owns user
}
```

The `consuming` convention means:

- ownership is transferred to the callee;
- the caller's binding is ended at the call;
- the callee must either transfer the value again or destroy it;
- returning the value transfers ownership to the caller.

The return type describes the ownership of the returned result. For example:

```text
fn name() -> &String  // borrowed result
fn take() -> String    // owned result
```

The return type alone does not indicate whether the receiver was consumed;
that is the role of `consuming`.

## 9. Ownership and Heap Values

Ownership and heap placement are two different things. A local value is already
owned:

```text
user := User {
    name: "Alice"
    age: 30
}
```

```text
User     = owned value
^User    = owned heap value
```

Heap values are created with the `^T` type directly, without any `make`:

```text
user := ^User {
    name: "Alice"
    age: 30
}
```

Construction is symmetric:

```text
User { ... }       // User inline
^User { ... }      // ^User heap-owned
```

A `^T` is an owned pointer to a `T` on the heap. It is automatically freed when
its ownership ends; safe code needs no matching `free`. There is no `Box<T>`
type and no `make` keyword.

If ownership is transferred, cleanup responsibility follows the value.

Example:

```text
fn create_user() -> ^User {
    return ^User {
        name: "Alice"
        age: 30
    }
}
```

The allocation created by `^User { ... }` is not destroyed before the returned
value is received by the caller.

A `^T` auto-dereferences. The heap is a storage choice, not a permanent
syntactic penalty:

```text
user: ^User

user.name        // not (*user).name
user.age = 31
user.birthday()
```

## 10. Copy, Clone, and Move

The language distinguishes three transfer modes:

```text
Copy    trivial implicit copy
Clone   explicit independent duplication
Move    ownership transfer
```

Primitive values are `Copy` and are implicitly copyable:

```text
x := 42
list.push(x)

// x still exists
```

Resource-owning values are moved by default:

```text
user := User { ... }
users.push(user)

// user moved
```

An independent duplication must be explicit with `clone()`:

```text
users.push(user.clone())

// user still valid
```

`clone()` must produce an independent value. It may allocate, so it is
deliberately visible in the source.

A `clone` followed by a move performs one duplication; the move itself copies
nothing:

```text
user
  |
 clone
  v
new User
  |
 move
  v
List
```

The exact rules for implicitly `Copy` structs are still open. The initial
direction is conservative: structs are move-only unless all their fields are
implicitly copyable or the type explicitly provides a clone implementation.

## 11. References and Lifetime Checks

`&T` is a read-only borrow. `&mut T` is a mutable borrow:

```text
fn display(user: &User) {
    println(user.name)
}

fn birthday(user: &mut User) {
    user.age += 1
}
```

Unlike Rust, `&mut` does not mean an exclusive borrow. Wev continues to allow
several mutable aliases when their lifetimes are valid:

```text
a := &mut user
b := &mut user
a.age += 1
b.age += 1
```

This is valid with respect to lifetime, but the programmer is responsible for
the resulting aliasing behavior and for concurrency.

A `&T` is independent of the memory location of its target. It can reference an
inline value, a heap-owned `^T`, or an element inside a collection:

```text
user := User {}       // inline
ref := &user

user := ^User {}      // heap-owned
ref := &user

ref := &users[0]      // element of a collection
```

A function taking `&User` does not need to know how the `User` is stored.

References are non-owning. The caller remains the owner. A referenced value is
never destroyed by the callee.

The compiler must reject obvious dangling references:

```text
fn invalid() -> &User {
    user := User { age: 30 }
    return &user // error: reference to local value escapes
}
```

The compiler tracks basic reference provenance:

```text
local value         -> reference cannot escape its scope
function parameter  -> reference may be returned if its source outlives it
heap owner          -> reference is valid while the owner is alive
```

Returning a reference derived from a parameter is allowed when valid:

```text
fn identity(user: &User) -> &User {
    return user
}
```

MVP lifetime rules should remain simple and conservative. Named lifetimes are
not required. The compiler may reject complex cases rather than introduce
lifetime annotations.

## 12. Mutation Through References and Owners

No separate `byref` parameter concept is needed. `&mut T` already covers the
"mutable reference to the caller's value" case uniformly, including for
primitives.

For a primitive:

```text
fn increment(value: &mut int) {
    value += 1
}

x := 10
increment(&mut x)

println(x) // 11
```

`int` is normally passed by value, but `&mut int` explicitly means "mutable
reference to the caller's `int`". A dedicated `byref value: int` parameter
would only duplicate this concept.

Owned pointers add a useful distinction: modifying the object behind an owner
versus modifying the owner itself.

### Modifying the object behind a `^User`

```text
fn birthday(user: &mut User) {
    user.age += 1
}

user := ^User {
    name: "Alice"
    age: 30
}

birthday(&mut user)
```

With auto-deref from `^User` to `&mut User`, the function can modify the
`User`, but it cannot change which `User` is owned by `user`:

```text
caller
user: ^User ───────────► User
                          age = 30
                            ↑
                            │
                       &mut User
```

### Modifying the owner itself

Passing a mutable reference to the `^User` allows reassigning the owner:

```text
fn replace_user(user: &mut ^User) {
    user = ^User {
        name: "Bob"
        age: 25
    }
}

user := ^User {
    name: "Alice"
    age: 30
}

replace_user(&mut user)

// user now points to Bob
```

```text
^User       owned heap User
&mut ^User  mutable reference to the owner
```

This is effectively what a `byref` parameter would provide.

### The old value is destroyed automatically

When `user = ^User { name: "Bob" }` replaces the value, the previous `Alice`
is destroyed automatically:

```text
user ──► Alice

replace

drop Alice
free Alice

user ──► Bob
```

No manual `free` is needed.

### One uniform rule

The rule is identical for primitives, structs, lists, and heap owners:

```text
fn set_int(x: &mut int)
fn set_user(x: &mut User)
fn set_heap_user(x: &mut ^User)
fn set_list(x: &mut List<User>)
```

There is nothing special to learn for primitives, which is another reason not
to introduce `byref`.

The Wev signature space can be summarized as:

```text
fn foo(x: int)           // value
fn foo(x: &int)          // readonly reference
fn foo(x: &mut int)      // mutable reference

fn foo(x: User)          // User value
fn foo(x: &User)         // observe a User
fn foo(x: &mut User)     // modify a User

fn foo(x: ^User)         // heap owner (passing semantics to be defined)
fn foo(x: &User)         // observe the User behind a ^User
fn foo(x: &mut User)     // modify the User behind a ^User
fn foo(x: &mut ^User)    // modify the owner/pointer itself
```

## 13. Enums and Results

Basic enums are part of the MVP:

```text
enum Result<T, E> {
    Ok(T)
    Err(E)
}

enum Option<T> {
    Some(T)
    None
}
```

Pattern matching syntax is useful but may be implemented immediately after the
first MVP parser. At minimum, enums must be constructible, inspectable, and
destroyed correctly when they contain owned values.

## 14. Collections and Aggregate Ownership

The first owning collection should be a dynamic list. `List<User>` is the
normal case: the list owns inline values directly in its storage.

```text
users := List<User>()

user := User {
    name: "Alice"
    age: 30
}

users.push(user)
```

Conceptually, `push(self: &mut List<T>, value: consuming T)` consumes its
element and moves it directly into the list's storage. There is no reason to
heap-allocate each object only because a collection must take ownership. After
insertion:

```text
display(user) // error: ownership moved into users
```

The ownership tree is:

```text
users
  +-- User Alice
```

When `users` is destroyed, it destroys all owned elements. The same rule
applies recursively to struct fields and enum payloads.

Collections have three distinct semantics:

```text
List<User>     // owns inline Users in its storage
List<^User>    // owns Users individually allocated on the heap
List<&User>    // does not own Users; stores only borrows
```

`List<^User>` is used when real indirection is wanted. Its buffer holds
owners/pointers, and each `User` keeps a stable address on the heap:

```text
users := List<^User>()

users.push(^User {
    name: "Alice"
    age: 30
})
```

`List<&User>` remains conceptually valid and may reference values that are
inline, heap-owned, or contained elsewhere. Its hard problem is lifetimes, not
storage location:

```text
refs := List<&User>()

{
    user := User {}
    refs.push(&user)
}

refs[0].display() // dangling
```

Supporting `List<&T>` and references stored in structs requires a more advanced
lifetime analysis. The MVP inclination is to postpone both rather than turn Wev
into a complex borrow checker.

The backend should ideally lower `push(user.clone())` (or a direct `User { ... }`
literal) by constructing the value in its destination storage rather than
materializing a temporary. This is an internal/IR optimization, not a syntax
rule.

An owner cannot be inserted into two owning collections without copying or
using a future shared-ownership abstraction:

```text
users_a.push(user)
users_b.push(user) // error: user has no remaining ownership
```

Shared ownership such as `Rc` or `Arc` is explicitly outside the MVP.

## 15. Traits

Traits express capabilities without introducing classes or inheritance:

```text
trait Printable {
    fn print()
}

impl Printable for User {
    fn print() {
        println(name)
    }
}
```

Traits do not contain instance state.

The MVP should support:

- declaring a trait;
- implementing a trait for a struct or enum;
- checking that required methods exist;
- using traits as static generic constraints if generics are implemented.

The MVP should not require:

- dynamic trait objects;
- implicit virtual dispatch;
- vtables;
- trait fields;
- complex trait composition;
- default methods, unless implementation is straightforward.

The preferred initial dispatch model is static dispatch and monomorphization.

## 16. Control Flow

Initial control-flow constructs:

```text
if condition {
    ...
} else {
    ...
}

while condition {
    ...
}

for item in collection {
    ...
}

return value
break
continue
```

Cleanup must be inserted correctly on every scope exit, including:

- normal block exit;
- `return`;
- `break` and `continue`;
- error propagation;
- branches;
- moved and partially initialized values.

## 17. Error Handling

The preferred error model is explicit values rather than exceptions:

```text
fn read_config(path: &String) -> Result<Config, IoError> {
    ...
}
```

An error-propagation operator may be included:

```text
config := read_config(path)?
```

Exceptions and implicit stack unwinding are outside the MVP.

## 18. C Interoperability

Headers can be included directly:

```text
include <stdio.h>
include "my_library.h"
```

The compiler does not need to parse every C macro or extension. It can copy
the include into the generated C file.

C declarations are written explicitly:

```text
extern "c" {
    fn printf(format: *const c_char, ...) -> i32
}
```

Raw pointers and unverifiable FFI calls require `unsafe`:

```text
unsafe {
    c_library_call(pointer)
}
```

The MVP should support:

- C headers;
- C function declarations;
- C-compatible primitive types;
- opaque C structs;
- linking `.c` files and libraries;
- compiler flags such as `-I`, `-L`, and `-l`.

The MVP does not need to understand arbitrary C macros, function-like macros,
compiler attributes, or complex preprocessor-generated types.

`include` addresses C headers only; Wev modules are imported with `import`
(Section 21).

## 19. Unsafe Code

`unsafe` is an explicit escape hatch for operations that the compiler cannot
verify:

- raw pointer dereference;
- pointer casts;
- manual allocation and deallocation;
- unchecked FFI contracts;
- assembly;
- manually asserted lifetime relationships.

Unsafe code is not required to obey the normal dangling-reference guarantee.
The safe language still guarantees that a safe reference cannot be destroyed
through a borrowed parameter.

## 20. Generated C

The C backend targets conservative C11:

- use `<stdint.h>`, `<stddef.h>`, `<stdbool.h>` and standard headers;
- avoid mandatory GNU extensions;
- avoid `typeof`, statement expressions, VLAs, and compiler-specific
  attributes in generated code;
- use explicit casts where needed;
- generate collision-resistant names with a Wev prefix;
- generate destructors for owning structs, enums, and collections;
- preserve source locations in generated comments when useful.

An owned struct should produce a cleanup function conceptually equivalent to:

```c
void Wev_User_drop(Wev_User *value);
```

The exact generated representation is an implementation detail as long as the
ownership behavior is preserved.

## 21. Modules, Collections, and Packages

Three related but distinct concepts:

- A **module** is a directory of `.wev` files. It provides a compilation unit
  (Section 22), a namespace, and a visibility boundary. This is the only unit
  named in source code: Wev code imports modules, never anything else.
- A **collection** is a bindable root directory, named at build time with
  `-collection:name=path` (Section 21.4). It is how the build resolves imports
  to modules on disk; source code never names a collection except as the first
  segment of an import path.
- A **package** is a distributable unit: a collection plus its metadata. It is
  a future concept (the manifest in Section 21.9) and never appears in source
  code. No import in Wev source refers to a package.

### 21.1 Directory = module

- Every directory is a module: it contains the `.wev` files directly in that
  directory, not in subdirectories.
- Files in the same directory belong to the same module and see each other
  directly, without imports or namespace prefixes.
- A subdirectory is a separate module.

### 21.2 Import syntax

An import is a dot-separated module path, without quotes:

```text
import math
import utils.io
import foo.math as m
```

- `math` names the module in the subdirectory `math` of the home collection
  root; `utils.io` names the module in `<root>/utils/io`.
- A path whose first segment matches a declared collection name resolves
  against that collection's directory instead (Section 21.4).
- The namespace bound in the importing file is the last path segment, or the
  alias when one is given.
- Because a path is a sequence of identifiers, every directory that is (or
  contains) a module must have a name that is a valid Wev identifier. There
  are no relative (`./`, `../`) imports; Section 21.5 explains how relocation
  works instead.
- Within the same directory, no import is needed.

### 21.3 Visibility

- `pub` exports a declaration: `pub fn`, `pub struct`, `pub enum`,
  `pub trait`, `pub const`, `pub impl`.
- Without `pub`, a declaration is private and visible only within its own
  module (its directory). Subdirectory modules cannot see private declarations.
- The public surface of a module is exactly its `pub` declarations. This is
  the module interface used by the cache (Section 22).

### 21.4 Resolution bases

Imports are resolved against the importing module's home collection:

- the project root is an implicit collection, whose root is the directory of
  the entry file;
- dependencies outside the project are declared at build time:

      wev build main.wev -collection:foo=/absolute/path/to/repo

  `-collection:name=path` binds `name` to a directory anywhere on disk;
  source code never contains physical paths outside the project. A cloned
  repo resolves its own internal imports against its own collection root.
- An import whose first segment matches a declared collection name resolves
  against that collection's directory; any other import resolves against the
  importing module's home collection root. So `foo.math` means "the `math`
  module inside the collection named `foo`", while `utils.io` means "the
  `utils/io` module inside my home collection root".

### 21.5 Module identity

There are no relative imports; every module is addressed from a collection
root by a path of identifiers. A module reached through a declared collection
and the same directory reached as the project root are therefore different
imports, but their identity is canonical:

- Module identity is canonical (realpath): the same directory reached through
  different paths is one module in the dependency graph and cache.
- Relocatability is provided by collections: to move a dependency elsewhere on
  disk, change the `-collection:name=path` binding at build time, never the
  source code. A cloned repo resolves its own internal imports against its own
  collection root.

### 21.6 Rules and errors

- Importing a nonexistent module is an error.
- An import path segment that is not a valid Wev identifier is a parse error.
- The import graph must be acyclic; circular imports are rejected.
- Two imports in the same file binding the same namespace are rejected.
- If a directory in the project root shares a name with a declared
  collection, the import is ambiguous and rejected.

### 21.7 Compilation model

- Each module compiles to one generated `.c`, one `.h`, and one `.o`
  (Section 22).
- Generated C names are prefixed with the module path for collision
  resistance (Section 20), e.g. `Wev_math_vec_pos`.
- A module's cache identity is its canonical collection plus its path within
  it.

### 21.8 C interop

`include` continues to address C headers (Section 18); `import` addresses
Wev modules only.

### 21.9 Future manifest

A manifest file may later wrap collections (`name = path`) without changing
the resolution model.

## 22. Incremental Build and Module Cache

This section records the intended strategy for fast incremental builds. It is a
design direction, not an MVP1 requirement. It presupposes the module system
described in Section 21.

### 22.1 Unit of compilation

A Wev module does not produce a self-contained `.c`. Almost any module has
external references that must be resolved by the C compiler:

- cross-module uses: types, functions, and methods defined in other modules;
- FFI: `include <stdio.h>` and other C headers and libraries;
- the Wev runtime (automatic drop, `String`, heap management), shared across
  modules;
- traits defined in another module that a local type implements;
- monomorphized generics whose bodies live in another module.

The expected model is therefore **one module = one generated `.c` + one
generated `.h` + one compiled `.o`**, assembled by the C linker. A module is
self-contained only in the rare case where it has no external references and
embeds a private copy of the runtime.

### 22.2 Interface, not ABI

The cache contract between modules is the **Wev-level interface**, not the
generated C header or ABI. A generated `.h` does not encode Wev ownership
semantics: `consuming` parameters, borrowed return provenance, and ownership
contracts. The later ownership passes need this information across module
boundaries, so the interface cache is a serialized Wev object derived from the
AST, not the generated header.

The interface of a module summarizes its public surface:

- exported type definitions and their layout;
- function and method signatures, including parameters, return type, and the
  ownership contract (`consuming`, `&`, `&mut`, returned borrows);
- trait definitions;
- `extern` declarations;
- `include` statements.

The interface is hashable and is the dependency key for consumers.

### 22.3 Cache layers

1. **Module interface**: the semantic summary above, stored per module. It is
   the only thing a consumer reads from its dependencies.
2. **Generated artifacts**: the `.c`, `.h`, and `.o` of a module, keyed by
   `hash(source) + hash(dependency interfaces) + compiler version + build
   flags`.
3. **Dependency graph**: the list of interfaces each module read, recorded from
   the last successful build (build-system style), so a rebuild can decide what
   to recompile without re-parsing every module.

### 22.4 Invalidation rules

A module is recompiled if and only if at least one of these changed:

- its own source;
- the interface of a dependency;
- the compiler version or build flags.

A change to a dependency's *implementation* that leaves its interface unchanged
does not propagate: only the changed module regenerates its `.c` and `.o`, and
the linker relinks. Consumers reuse their cached artifacts.

### 22.5 Generics

Monomorphization makes a consumer depend on the *implementation* of a generic,
not only its interface. Two escape routes are considered:

- emit generic bodies in the generated header (template style), making them
  part of the interface, at the cost of more invalidation and code bloat;
- exclude modules containing generics from the cache fast path.

Generics are outside MVP1, so this decision can be made when generics are
designed.

### 22.6 AST consequence

The flat-array AST design (nodes and tokens stored as contiguous arrays
referenced by indices, never by pointers, see `ast_design.md`) makes the
interface trivially derivable: it is a serialized subset of the exported nodes,
produced by the same mechanism as any future AST cache. No pointer fix-ups are
needed when writing and reloading the cache.

## 23. Compiler Pipeline

The intended pipeline is:

```text
source
  -> lexer
  -> parser
  -> module resolution
  -> name resolution
  -> type checking
  -> move and ownership analysis
  -> reference escape analysis
  -> destructor/drop insertion
  -> C code generation
  -> tcc/clang/gcc
```

The ownership checker must be flow-sensitive enough to handle branches and
early returns, but it does not need Rust's full borrow-checking model.

## 24. MVP1 Acceptance Example

The following example represents the minimum useful vertical slice:

```text
include <stdio.h>

struct User {
    name: String
    age: i32
}

impl User {
    fn birthday() {
        age += 1
    }

    fn display() {
        println(name)
    }

    consuming fn into_name() -> String {
        return name
    }
}

fn main() {
    users := List<User>()

    user := User {
        name: "Alice"
        age: 30
    }

    user.birthday()
    users.push(user)
    users[0].display()
}
```

The compiler must reject at least:

```text
fn invalid() -> &User {
    user := User { age: 30 }
    return &user
}
```

```text
fn invalid_move() {
    user := User { age: 30 }
    consume_user(user)
    user.display()
}
```

## 25. MVP1 Exclusions

The following are deliberately postponed:

- classes and inheritance;
- garbage collection;
- reference counting and shared ownership;
- named lifetimes;
- Rust-style exclusive mutable borrowing;
- async/await;
- closures with escaping captures;
- macros;
- reflection;
- C++/CUDA/Objective-C backends;
- full C header parsing;
- dynamic trait objects;
- advanced generic specialization;
- incremental build and module caching (see Section 22);
- optimizer-driven heap-to-stack promotion of `^T` allocations.

## 26. Open Decisions

The following decisions remain to be made before implementation is considered
stable:

1. Are semicolons optional everywhere or only at line boundaries?
2. Is `String` a built-in type or a standard-library type implemented in Wev?
3. Are references allowed as fields in safe structs, or only as local values and
   parameters in MVP1?
4. Should `List<&T>` lifetime checking be supported immediately or postponed?
5. What is the exact overflow behavior for `i32` and other signed integers?
6. Should traits and generic constraints be part of the first compiler vertical
   slice or added after structs and ownership work?
7. Which C compiler is the reference backend for generated-code correctness?
8. Should the C backend target C99 or C11?
9. Resolved: the module and import system for cross-file compilation is
   designed in Section 21. Incremental builds and module caching remain
   postponed (Section 22).
10. Is `Wev` the final public name after availability and trademark checks?
11. Resolved: `^T` is the owned-heap-value glyph (Section 2) and `*T` the raw /
    unmanaged pointer. `^` is therefore reserved; a future bitwise XOR operator
    will not reuse this glyph.
