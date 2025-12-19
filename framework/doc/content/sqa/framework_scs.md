!template load file=sqa/scs.md.template

[!ac](MOOSE) uses [ClangFormat](https://clang.llvm.org/docs/ClangFormat.html) for formatting all
C++ code in the repository. The application of the format is enforced and checked when the a
code is contributed, see [framework/contributing.md] for more details. The
configuration file for the formatting is provided in the
[.clang-format](https://github.com/idaholab/moose/blob/devel/.clang-format) file. If clang is
installed, the following command will automatically format code changed between your current branch
and the supplied `<branch>` (if omitted, defaults to HEAD).

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

The ideal solution would be to match the type you are looping over, but this is slightly more
difficult than just using `decltype` due to qualifiers like `const`. In MOOSE we have solved this for
you with `MooseIndex`:

```C++
  // Looping to a scalar index
  for (MooseIndex(n_nodes) i = 0; i < nodes; ++i)
    // ... Do something with index i

  // Looping to a container size (where range-for isn't sufficient)
  for (MooseIndex(vector) i = 0; i < vector.size(); ++i)
    // ... Do something with index i
```

In addition, a few convenience methods are available to help generate integer ranges:

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

Remember `using` also works with template.

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

## Free function v.s. member function

When adding a new function, please carefully decide whether to make it a free function (i.e., outside the scope of the class) or a member function.

The core rule is

> Make it a member function only if it fundamentally depends on the object’s invariants or representation.

Define a member when the operation:

- Maintains or relies on class invariants

  ```C++
  class Matrix
  {
  public:
    void resize(int m, int n); // must keep internal consistency
  };
  ```

  If the function mutates internal state, must preserve class invariants, or requires intimate knowledge of representation, make it a member function.

- Is conceptually “owned” by the type

  ```C++
  particle.update(dt);
  tensor.transpose();
  ```

  Ask: “Would users naturally discover this by typing obj.?” If so, make it a member function.

- Needs privileged access: If the function would otherwise require: friendship, access to private data, or bypassing encapsulation, make it a member (or a carefully chosen friend).

Use a free function for

- Symmetric operations

  ```C++
  dot(a, b);
  distance(p, q);
  misorientation(g1, g2);
  ```
  If no operand is conceptually dominant: Avoid `a.dot(b)` and prefer `dot(a, b)`.

- Algorithms over data

  ```C++
  norm(v);
  det(A);
  project(u, v);
  ```

  If it takes an object as input, produces a value, and does not modify the object, make it a free function. This aligns with the STL design: containers own storage; algorithms are free.

- Extensibility across types

  ```C++
  template<class T>
  auto norm(const T & x);
  ```

  Free functions allow generic programming, ADL-based customization, extension to foreign types. Member functions cannot be added retroactively.

- Operations that conceptually sit “between” types

  ```C++
  intersect(mesh, plane);
  assemble(system, element);
  ```

  These don’t belong to either object alone.

In summary, when making a decision, ask these questions in order:

1. Does this operation mutate the object or enforce invariants? Yes → member
2. Is it conceptually “owned” by the object? Yes → member
3. Is the operation symmetric or algorithmic? Yes → free function
4. Do I want/expect others to extend this behavior for new types? Yes → free function
5. If you’re unsure → default to free function.

As a reminder, helper free functions defined only in the translation unit (i.e., `.C` files) should be marked as `static` or encapsulated in an anonymous namespace to force static linking.

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

Code reviews may request access-level reductions if members are more visible than necessary.

## Generic Programming and Compile-Time Constants

Prefer generic programming and compile-time constants when they improve correctness, expressiveness, or performance. Use templates to express behavior that is truly type- or dimension-independent.

Prefer:

- Function and class templates for algorithms that operate uniformly across types
- `auto` and template parameter deduction to reduce redundancy
- Concepts (not available yet in C++17) or `static_assert` to document and enforce template requirements

Avoid:

- Templates used only to avoid writing a small amount of code
- Deep or obscure template metaprogramming that obscures intent
- Encoding runtime variability as template parameters without a clear benefit

Templates should make code more general and clearer, not harder to understand.

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
- Thou shalt use range-based loops or `MooseIndex()` based loops for iteration.
