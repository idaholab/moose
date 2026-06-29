# MOOSE Code Standard - Review Checklist

Condensed from `framework/doc/content/sqa/framework_scs.md`. This is what to check by eye;
formatting (ClangFormat/Black/whitespace) is enforced by CI, so only remind the author to run
`git clang-format <base>` / `black .` rather than flagging each formatting nit.

Organized highest-value first.

## Const-correctness (strict - treated as a design issue, not cosmetic)
- Function parameters that are not modified are marked `const`.
- Member functions that don't modify observable state are marked `const`.
- Return `const` references when mutation isn't intended.
- No `const_cast` to remove constness except in narrowly justified, documented cases.
- Logically-const-but-unmarked code should be called out - it weakens the API contract and
  blocks compiler diagnostics/optimization.

## Access control (helps API review)
- Default to `private`. Promote to `public` only for safe, supported, invariant-preserving API.
- `protected` is for documented inheritance extension points, not general access. Avoid
  `protected` data members - prefer `protected` virtual/helper functions.
- Within a class definition, order members `public` -> `protected` -> `private`.
- Prefer composition over inheritance when behavior can be delegated.

## Naming and files
- `ClassName` - CamelCase. The `.h` and `.C` filenames must match the class name exactly.
- `methodName()` - camelCase, leading lowercase.
- `_member_variable` - leading underscore, all lowercase, underscores between words.
- `local_variable` - lowercase, starts with a letter, underscores between words.
- One class per `.h`/`.C` pair (helper/non-exposed classes are the exception).
- Headers in `include/` (or a subdir); sources in `src/` (or a subdir).

## Prefer modern C++ over C constructs
- `nullptr` (not `NULL`/`0`); `static_cast`/`const_cast`/`reinterpret_cast` (not C-style casts).
- `constexpr`/`const` over `#define` for constants; `enum class` over plain `enum`.
- `std::array`/`std::vector`/`std::span` over raw arrays + pointer arithmetic.
- RAII (`std::unique_ptr`, `std::lock_guard`) over manual resource management.
- `<algorithm>`/`<numeric>` over hand-written loops where it reads better.

## Header hygiene and includes
- Forward-declare in headers where possible; include full definitions in the `.C`.
  (Exception: when forcing end users to include multiple files to complete child definitions.)
- Only include what's necessary in headers.
- Non-system includes use quotes with a single space: `#include "Foo.h"`; system includes use
  angle brackets: `#include <vector>`.

## Functions, lambdas, loops
- New function placement: non-static member (needs/owns object state) vs static member
  (belongs to the type, e.g. factories/`validParams`) vs free function (symmetric/algorithmic/
  cross-cutting). When unsure, default to free function or static member for namespacing.
- TU-local helper free functions go in an anonymous namespace.
- Lambdas: list captures explicitly rather than blanket `[=]`/`[&]` where practical.
- Avoid signed/unsigned mismatch in index loops; use `make_range(n)`, `make_range(n1, n2)`, or
  `index_range(v)` from `libmesh/int_range.h` instead of `for (auto i = 0; i < v.size(); ++i)`.
- Prefer structured bindings when unpacking tuples/pairs/aggregates, especially in loops.
- Prefer `using` over `typedef` for aliases.

## `auto`
- Use `auto` unless it hurts readability; never in a function/method declaration (signatures
  stay explicit). Ensure variables still have good, descriptive names.

## The Code Commandments (frequent review catches)
- Use references instead of pointers when the referent outlives the user.
- Return pointers for pointer-stored objects, references for reference-stored objects.
- Every class with virtual methods has a `virtual` destructor.
- Use `override` on overridden virtuals; do not also write `virtual` (redundant).
- `std::make_shared<T>()` / `std::make_unique<T>()` for new shared/unique allocations.
- Use `std::move()` for efficiency where applicable.
- All function definitions live in `.C` files (exceptions: inline-for-speed and templates).
- Avoid global variables.
- Never rely on `operator[]` accidental insertion into a `std::map` on the right-hand side
  unless it's proven it can't fail.

## Python (when `.py` files change)
- Same spirit as above, with: 4-space indentation; members named `self.public_member`,
  `self._protected_member`, `self.__private_member`.
