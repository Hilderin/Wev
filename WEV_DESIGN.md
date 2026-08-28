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

Wev uses values by default. Heap allocation is explicit with `make`, but its
cleanup is automatic.

The language protects the most important ownership invariants:

- An owned allocation has one owner at a time.
- Moving ownership invalidates the source binding.
- A reference does not own or destroy its target.
- A reference cannot outlive its target when the compiler can determine this.
- Copying a resource-owning value is explicit.

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
```

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
- owned heap values;
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

Normal methods borrow their receiver and do not take ownership. The receiver
may be mutated, consistent with Wev's permissive aliasing model.

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

## 9. Ownership and `make`

Local values are created normally:

```text
user := User {
    name: "Alice"
    age: 30
}
```

Heap values are created explicitly with `make`:

```text
user := make(User {
    name: "Alice"
    age: 30
})
```

`make` creates an owned heap value. It does not require a visible `Box<T>` type
and does not require a matching `free` in safe code.

The owner is automatically cleaned up when its ownership ends. If ownership is
transferred, cleanup responsibility follows the value.

Example:

```text
fn create_user() -> User {
    return make(User {
        name: "Alice"
        age: 30
    })
}
```

The allocation created by `make` is not destroyed before the returned value is
received by the caller.

## 10. Copy and Move

Primitive values are implicitly copyable:

```text
a := 42
b := a
```

Resource-owning values are moved by default:

```text
user2 := user
display(user) // error: user was moved
```

An independent copy must be explicit:

```text
user2 := user.copy()
```

`copy()` must produce an independent value. For a resource-owning struct,
copying is only available when the type defines a valid copy operation.

The exact rules for implicitly copyable structs are still open. The initial
direction is conservative: structs are move-only unless all their fields are
implicitly copyable or the type explicitly provides a copy implementation.

## 11. References and Lifetime Checks

References are non-owning:

```text
fn birthday(user: &User) {
    user.age += 1
}

fn display(user: &User) {
    println(user.name)
}
```

The caller remains the owner. A referenced value is never destroyed by the
callee.

The compiler must reject obvious dangling references:

```text
fn invalid() -> &User {
    user := User { age: 30 }
    return &user // error: reference to local value escapes
}
```

The compiler tracks basic reference provenance:

```text
local value       -> reference cannot escape its scope
function parameter -> reference may be returned if its source outlives it
heap owner         -> reference is valid while the owner is alive
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

Mutable aliases are allowed. Wev does not enforce Rust-style exclusive
mutable borrowing in the MVP:

```text
a := &user
b := &user
a.age += 1
b.age += 1
```

This is valid with respect to lifetime, but the programmer is responsible for
the resulting aliasing behavior and for concurrency.

An optional read-only reference syntax may be added later:

```text
&const User
```

## 12. Enums and Results

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

## 13. Collections and Aggregate Ownership

The first owning collection should be a dynamic list:

```text
users := make(List<User>())

user := make(User {
    name: "Alice"
    age: 30
})

users.push(user)
```

`push` consumes its element. After insertion:

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

Collection categories:

```text
List<T>     // owns its elements
List<&T>    // stores non-owning references, lifetime checked
List<*T>    // stores raw pointers, unsafe
```

An owner cannot be inserted into two owning collections without copying or
using a future shared-ownership abstraction:

```text
users_a.push(user)
users_b.push(user) // error: user has no remaining ownership
```

Shared ownership such as `Rc` or `Arc` is explicitly outside the MVP.

## 14. Traits

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

## 15. Control Flow

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

## 16. Error Handling

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

## 17. C Interoperability

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

## 18. Unsafe Code

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

## 19. Generated C

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

## 20. Compiler Pipeline

The intended pipeline is:

```text
source
  -> lexer
  -> parser
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

## 21. MVP1 Acceptance Example

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
    users := make(List<User>())

    user := make(User {
        name: "Alice"
        age: 30
    })

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
    user := make(User { age: 30 })
    consume_user(user)
    user.display()
}
```

## 22. MVP1 Exclusions

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
- optimizer-driven stack promotion of `make` allocations.

## 23. Open Decisions

The following decisions remain to be made before implementation is considered
stable:

1. Are semicolons optional everywhere or only at line boundaries?
2. Is `&T` always mutable, with `&const T` added later for read-only access?
3. What exact syntax should `make` use for collections and generic types?
4. Is `String` a built-in type or a standard-library type implemented in Wev?
5. Are references allowed as fields in safe structs, or only as local values and
   parameters in MVP1?
6. Should `List<&T>` lifetime checking be supported immediately or postponed?
7. What is the exact overflow behavior for `i32` and other signed integers?
8. Should traits and generic constraints be part of the first compiler vertical
   slice or added after structs and ownership work?
9. Which C compiler is the reference backend for generated-code correctness?
10. Is `Wev` the final public name after availability and trademark checks?
