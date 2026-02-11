!template load file=sqa/scs.md.template

## C++ File Formatting

[!ac](MOOSE) uses [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) for formatting all
C++ code in the repository. The application of the format is enforced and checked when the a
code is contributed, see [framework/contributing.md] for more details. The
configuration file for the formatting is provided in the
[.clang-format](https://github.com/idaholab/moose/blob/devel/.clang-format) file. If clang is
installed, the following command will automatically format code changed between your current branch
and the supplied `<branch>` (if omitted, defaults to HEAD):

```
git clang-format [<branch>]
```

The automated testing that occurs on all Pull Requests will produce a "diff" of changes needed to
conform with the standard.

General style guidelines include:

-  Single spacing around all binary operators
-  No spacing around unary operators
-  No spacing on the inside of brackets or parenthesis in expressions
-  Avoid braces for single statement control statements (i.e for, if, while, etc.)
-  C++ constructor spacing is demonstrated in the bottom of the example below

## Python File Formatting

[!ac](MOOSE) uses [Black](https://black.readthedocs.io/en/stable/) for formatting all
Python code in the repository. The application of the format is enforced and checked when the a
code is contributed, see [framework/contributing.md] for more details. The
configuration file for the formatting is provided in the
[pyproject.toml](https://github.com/idaholab/moose/blob/devel/pyproject.toml) file. If black is
installed, the following command will automatically format all python code when ran
from the root of the [!ac](MOOSE) repository:

```
black .
```

## File Layout

- Header files should have a ".h" extension
- Header files always go either into "include" or a sub-directory of "include"
- C++ source files should have a ".C" extension
- Source files go into "src" or a subdirectory of "src".

## Files

Header and source file names must match the name of the class that the files define. Hence, each set of .h and .C files
should contain code for a single class. Helper classes or classes not exposed to the user are an exception to this rule.

- src/ClassName.C
- include/ClassName.h

## Naming

- `ClassName` Class names utilize camel case, note the .h and .C filenames must match the class name.
- `methodName()` Method and function names utilize camel case with the leading letter lower case.
- `_member_variable` Member variables begin with underscore and are all lower case and use underscore between words.
- `local_variable` Local variables are lowercase, begin with a letter, and use underscore between words

## Use of C++ `auto` keyword

Use `auto` unless it complicates readability, but do not use it in a function or method declaration.
Make sure your variables have +good+ names when using auto!

```C++
  auto dof = elem->dof_number(0, 0, 0);
  auto & obj = getSomeObject();
  auto & elem_it = mesh.active_local_elements_begin();
  auto & item_pair = map.find(some_item);

  // Cannot use reference here
  for (auto it = obj.begin(); it != obj.end(); ++it)
    doSomething();

  // Use reference here
  for (auto & obj : container)
    doSomething();
```

## Const-correctness

We expect contributions to follow strict const-correctness.

- Use const to accurately express intent, invariants, and ownership.
- Mark function parameters const whenever they are not modified.
- Mark member functions const whenever they do not modify observable object state.
- Prefer returning const references when mutation is not intended.
- Avoid casting away const (e.g., `const_cast`) except in narrowly justified, well-documented cases.

Const-correctness is not cosmetic. It:

- Improves API clarity and self-documentation
- Enables safer refactoring and compiler diagnostics
- Allows the compiler to reason more effectively about aliasing and optimization
- Prevents accidental state mutation, especially in generic and templated code

Code that is logically const but not marked const will generally be treated as a design issue during review.

## Index Variables in Looping Constructs

In keeping with good `auto` practices, it would be really nice if we could use `auto` for
loop indices in places where range loops aren't convenient or possible. Unfortunately, the most
natural code one would like to write yields a warning with our default compiler flags:

```C++
  for (auto i = 0; i < v.size(); ++i) // DON'T DO THIS!

  warning: comparison of integers of different signs: 'int' and 'std::__1::vector<int, std::__1::allocator<int> >::size_type' (aka 'unsigned long')
```

!alert note
This warning stems from the fact that numeric literals in C++ are treated as "signed ints". There are
suffixes that can be applied to literals to force the "correct" type, but are you sure you really
know the correct type? If you are thinking "unsigned int" you are in the majority and unfortunately
also wrong. The right type for a container is `::size_type (aka 'unsigned long')`. See the error
message once more above.

A few convenience methods are available to help generate integer ranges:

```C++
#include "libmesh/int_range.h"

// Looping through n1, n1+1, n1+2, ..., n2-1
for (const auto i : make_range(n1, n2))
  // ... Do something with index i

// Looping through 0, 1, 2, ..., n-1
for (const auto i : make_range(n))
  // ... Do something with index i

// Looping through 0, 1, 2, ..., v.size()-1
for (const auto i : index_range(v))
  // ... Do something with index i
```

## Structured binding

Prefer structured binding when unpacking aggregate types and tuple-like objects.

An example for aggregate type structured binding:

```C++
struct S
{
  int x;
  double y;
};

S s{1, 2.0};

// Bind by copying
auto [a, b] = s;

// Bind by reference
const auto & [a_ref, b_ref] = s;
```

An example for tuple-like object binding,

```C++
std::tuple<int, Real, Point> t = {1, 0.5, Point(0.1, 0.2, 0.3)};
const auto & [a, b, c] = t;
```

Tuple-like object includes `std::pair`, `std::tuple`, `std::array` etc.

Using structured binding for unpacking sub-objects in a loop makes the intention very clear, i.e.,

```C++
std::vector<std::tuple<dof_id_type, unsigned int, BoundaryID>> my_vec;
for (const auto & [elem_id, side, bnd] : my_vec)
  // ...
```

## Aliasing

Consider using `using` (over `typedef`) to define type aliases to improve code readability:

```C++
std::vector<std::tuple<dof_id_type, unsigned int, BoundaryID>> my_vec;

// Simplify code and clarify intention by defining an alias
using ElemSide = std::tuple<dof_id_type, unsigned int, BoundaryID>;
std::vector<ElemSide> my_vec2;
```

Remember that `using` also works with templates.

## Lambdas

List captured variables (by value or reference) in the capture list explicitly where possible.

```C++
  std::for_each(container.begin(), container.end(), [=local_var](Foo & foo) {
    foo.item = local_var;
  });
```

## Other C++ Notes

- Use the `override` keyword on overridden `virtual` methods; do not mark the overriding method as `virtual` as it is redundant
- Use `std::make_shared<T>()` when allocating new memory for shared pointers
- Use `std::make_unique<T>()` when allocating new memory for unique pointers
- Make use of std::move() for efficiency when possible

## Variable Initialization

When creating a new variable use these patterns:

```C++
unsigned int i = 4;  // Built-in types
SomeObject junk(17); // Objects
SomeObject * stuff = new SomeObject(18); // Pointers
```

## Trailing Whitespace and Tabs

Tabs and trailing whitespace are not allowed in the C++ in the repository. Running the
following one-liner from the root directory of your repository will format the code correctly.

```bash
find . -name '*.[Chi]' -or -name '*.py' | xargs perl -pli -e 's/\s+$//'
```

## Prefer C++ constructs over C constructs

Use modern C++ language features and standard library facilities in preference to legacy C-style constructs. C++ provides stronger type safety, better abstraction, and clearer intent. New code should reflect these advantages.

Prefer:

- `nullptr` over `NULL` or `0`
- `static_cast`, `const_cast`, `reinterpret_cast` over C-style casts
- `constexpr` and `const` over preprocessor macros
- `enum class` over unscoped `enum`
- `std::array`, `std::vector`, `std::span` over raw arrays and pointer arithmetic
- RAII types (e.g., `std::unique_ptr`, `std::lock_guard`) over manual resource management
- `<algorithm>` and `<numeric>` over hand-written loops where appropriate

## C++ Includes

Only include things that are absolutely necessary in header files. Please use forward declarations
when possible.

```C++
// Forward declarations
class Something;
```

All non-system includes should use quotes with a single space between `include` and the filename.

```C++
#include "LocalInclude.h"
#include "libmesh/libmesh_include.h"

#include <system_library>
```

## Free function vs. member function vs. static member function

When adding a new function, carefully decide whether it should be

- a **non-static member function** (operates on an object),
- a **static member function** (conceptually belongs to a type, but not to an instance), or
- a **free function** (algorithm or operation external to any one type).

### Use a **non-static member function** when the operation:

- Maintains or relies on class internal data that must be kept consistent

  ``` cpp
  class Matrix
  {
  public:
    void resize(int m, int n); // must keep internal consistency
  };
  ```

  If the function mutates internal state or requires intimate knowledge of representation, it should be a non-static member.

- Is conceptually "owned" by a specific object

  ``` cpp
  particle.update(dt);
  tensor.transpose();
  ```

  Ask: "Would users naturally expect to discover this via `obj.`?" If yes, make it a non-static member.

- Needs privileged access to object internals

  If it would otherwise require friendship or breaking encapsulation, it usually belongs as a non-static member (or, rarely, a tightly controlled friend).

### Use a **static member function** when the operation:

- Conceptually belongs to the type, but not to any one instance

  ``` cpp
  class Tensor
  {
  public:
    static Tensor identity(unsigned int dim);
    static Tensor fromEulerAngles(double a, double b, double c);
  };
  ```

  These are type-level operations: factories, validators, canonical constructors, or utilities closely tied to the abstraction.

- Provides behavior naturally namespaced under the class

  ``` cpp
  Elem::build(type, dim);
  FEProblemBase::validParams();
  ```

  Static members improve discoverability, communicate conceptual ownership, and avoid polluting surrounding namespaces, even though no object data is used.

- Logically groups algorithms that are tightly coupled to a type's meaning

  Especially in large frameworks like MOOSE, static members can act as a *semantic namespace* that documents intent better than a free function.

  Think of static members as answering:

  > "Does this function describe the concept of this class, even though it doesn't operate on an instance?"

  If yes, a static member is often the right choice.

### Use a **free function** when the operation is:

- Symmetric

  ``` cpp
  dot(a, b);
  distance(p, q);
  misorientation(g1, g2);
  ```

  If no operand is conceptually dominant, e.g. avoid `a.dot(b)`.

- An algorithm over data

  ``` cpp
  norm(v);
  det(A);
  project(u, v);
  ```

  If it takes objects as inputs and produces a value, prefer a free function (STL style).

- Meant to be extensible across unrelated types

  ``` cpp
  template <class T>
  auto norm(const T & x);
  ```

  Free functions enable generic programming, argument-dependent lookup (ADL) customization, and retroactive extension to foreign types.

- Conceptually "between" types

  ``` cpp
  intersect(mesh, plane);
  assemble(system, element);
  ```

  These don't belong to either type alone.

### Decision checklist

1.  Does the function need access to object's internal data/state? Yes → non-static member
2.  Does it conceptually belong to the type, but not an instance? Yes → static member
3.  Is it symmetric, algorithmic, or cross-cutting? Yes → free function
4.  Should users be able to extend it for new types via overloading/ADL? Yes → free function
5.  Unsure? → default to free function (or static member if discoverability/namespacing is the primary concern)

### Implementation note

Helper free functions that are local to a translation unit (`.C` files) should be placed in an anonymous namespace (or marked `static`) to avoid external linkage.

## Access control: public, protected, and private

Choose the narrowest access level that correctly expresses intent.

Public members and methods form the stable API of a class. They define what users are allowed to rely on. Use `public` only for:

- Operations that are safe, well-defined, and intended for external use
- Methods that preserve class invariants
- Behavior you are willing to support long-term

Public interfaces should be minimal, const-correct, and clearly documented. Adding something to `public` is a commitment.

Prefer `private` by default. Use `private` for:

- Representation details and internal state
- Helper functions and implementation mechanics
- Anything that should not be relied upon by derived classes or external code

Private members allow internal refactoring without breaking user assumptions and are the strongest tool for enforcing invariants.

`protected` exists to support controlled customization via inheritance, not general access. Use `protected` only when:

- A derived class must override or extend behavior
- The base class explicitly documents a customization or extension point
- The invariant contract between base and derived classes is clear

Avoid exposing data members as `protected`. Prefer `protected` virtual functions or `protected` helper methods instead.

If a member is not intended to be used or overridden by derived classes, it should **not** be `protected`.

In summary:

- Default to `private`
- Promote to `public` only when there is a clear, justified use case
- Treat `protected` members as implementation details meant only for subclasses, not as part of the class’s public interface
- Consider composition as an alternative to inheritance, especially when behavior can be delegated rather than extended.

Code reviews may request access-level reductions if members are more visible than necessary. In MOOSE, the preferred access order should be, whenever possible,

- public
- protected
- private

in the class definition. Following this convention helps expedite API review.

## Generic programming and compile-time design

Prefer generic programming and compile-time constructs when they improve correctness, extensibility, or performance, or when they enable significantly cleaner and safer downstream code. Use templates to express behavior that is truly type-, dimension-, or policy-independent.

!alert note
Generic code often forms infrastructure: it may be more complex than its uses, but should make user code simpler, safer, and more expressive.

Prefer

- **Function and class templates** for algorithms and abstractions that operate uniformly across types, dimensions, or policies
- **Compile-time constants and types** to express invariants, dimensions, and configuration that are fundamental to the model
- **`auto` and template deduction** to reduce redundancy
- **Explicit documentation of template requirements**: Use `static_assert`, type traits, and clear comments to state what a template expects and guarantees.
- **Encapsulation of template complexity**: Isolate nontrivial metaprogramming behind small, stable interfaces.

Be cautious about

- **Templates used only to avoid small amounts of duplication**, without a clear gain and with decreased readability
- **Encoding inherently runtime variability as template parameters** without a strong semantic or performance reason
- **Letting template machinery leak into user code**

Not all valuable templates are simple. Foundational libraries (e.g. the STL, MetaPhysicL, expression-template frameworks) are often hard to read internally, yet dramatically improve downstream code. In that vein, do not reject generic designs solely because their implementation is nontrivial. Do require that unavoidable complexity be justified, contained, documented, and tested.

Prefer compile-time constants when values are known at compile time and semantically fixed.

Use:

- `constexpr` or `consteval` for values and functions known at compile time
- Non-type template parameters (e.g., dimensions, tensor ranks, fixed sizes) when they affect layout, correctness, or optimization
- `std::integral_constant` or `constexpr` variables instead of macros

Avoid:

- `#define` for constants or logic
- Encoding configuration or runtime choices as compile-time constants without justification

Compile-time constants should represent invariants, not tunable parameters.

## Documentation

- In source documentation should be extensive, designed toward the software developer, and be
  formatted for the use of [Doxygen](https://www.doxygen.nl/index.html).

## Python

Where possible, follow the above rules for Python.  The only modifications are:

1. Four spaces are used for indenting and
2. Member variables should be named as follows:

```python
class MyClass:
    def __init__(self):
        self.public_member
        self._protected_member
        self.__private_member
```

## Code Commandments

- Use references instead of pointers whenever possible.
    - i.e., this object lives for a shorter period of time than the object it needs to refer to does
- Methods should return pointers to objects if returned objects are stored as pointers and references if returned objects are stored as references.
- When creating a new class:
    - Include dependent header files in the *.C file whenever possible (i.e. the header uses only references or pointers in it's various declarations) and use forward declarations in the header file as needed.
    - One exception is when doing so would require end users to include multiple files to complete definitions of child objects (Errors typically show up as "incomplete class" or something similar during compilation).
- Avoid using a global variable when possible.
- Every destructor must be virtual.
- All function definitions should be in *.C files.
    - The only exceptions are for inline functions for speed and templates.
- Thou shalt not commit accidental insertion in a std::map by using brackets in a right-hand side operator unless proof is provided that it can't fail.
