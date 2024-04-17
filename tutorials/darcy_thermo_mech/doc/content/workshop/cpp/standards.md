# MOOSE C++ Standard

!---

## Clang Format

MOOSE uses "clang-format" to automatically format code:

```bash
git clang-format branch_name_here
```

- Single spacing around all binary operators
- No spacing around unary operators
- No spacing on the inside of brackets or parenthesis in expressions
- Avoid braces for single statement control statements (i.e for, if, while, etc.)
- C++ constructor spacing is demonstrated in the bottom of the example below

!---

## File Layout

- Header files should have a ".h" extension
- Header files always go either into "include" or a sub-directory of "include"
- C++ source files should have a ".C" extension
- Source files go into "src" or a subdirectory of "src".

!---

## Files

Header and source file names must match the name of the class that the files define. Hence, each set
of .h and .C files should contain code for a single class.

- src/ClassName.C
- include/ClassName.h

!---

## Naming

- `ClassName` Class names utilize camel case, note the .h and .C filenames must match the class name.
- `methodName()` Method and function names utilize camel case with the leading letter lower case.
- `_member_variable` Member variables begin with underscore and are all lower case and use underscore
  between words.
- `local_variable` Local variables are lowercase, begin with a letter, and use underscore between words

!---

## Example Code

Below is a sample that covers many (not all) of our code style conventions.

```cpp
namespace moose // lower case namespace names
{
// don't add indentation level for namespaces

int // return type should go on separate line
junkFunction()
{
  // indent two spaces!
  if (name == "moose") // space after the control keyword "if"
  {
    // Spaces on both sides of '&' and '*'
    SomeClass & a_ref;
    SomeClass * a_pointer;
  }

  // Omit curly braces for single statements following an if the statement must be on its own line
  if (name == "squirrel")
    doStuff();
  else
    doOtherStuff();

  // No curly braces for single statement branches and loops
  for (unsigned int i = 0; i < some_number; ++i) // space after control keyword "for"
    doSomething();

  // space around assignment operator
  Real foo = 5.0;

  switch (stuff) // space after the control keyword "switch"
  {
    // Indent case statements
    case 2:
      junk = 4;
      break;
    case 3:
    { // Only use curly braces if you have a declaration in your case statement
      int bar = 9;
      junk = bar;
      break;
    }
    default:
      junk = 8;
  }

  while (--foo) // space after the control keyword "while"
    std::cout << "Count down " << foo;
}

// (short) function definitions on a single line
SomeClass::SomeFunc() {}

// Constructor initialization lists can all be on the same line.
SomeClass::SomeClass() : member_a(2), member_b(3) { }

// Four-space indent and one item per line for long (i.e. won't fit on one line) initialization list.
SomeOtherClass::SomeOtherClass()
  : member_a(2),
    member_b(3),
    member_c(4),
    member_d(5),
    member_e(6),
    member_f(7),
    member_g(8),
    member_h(9)
{ // braces on separate lines since func def is already longer than 1 line
}

} // namespace moose
```

!---

## Using auto

Using `auto` improves code compatibility.  Make sure your variables have
+good+ names so auto doesn't hurt readability!

```cpp
auto dof = elem->dof_number(0, 0, 0);  // dof_id_type, default 64-bit in MOOSE
int dof2 = elem->dof_number(0, 1, 0);  // dof2 is broken for 2B+ DoFs
auto & obj = getSomeObject();
auto & elem_it = mesh.active_local_elements_begin();
auto & [key, value] = map.find(some_item);

// Cannot use reference here
for (auto it = container.begin(); it != container.end(); ++it)
  doSomething(*it);

// Use reference here
for (auto & obj : container)
  doSomething(obj);
```

Function declarations should tell programmers what return type to
expect; `auto` is only appropriate there if it is necessary for a
generic algorithm.

!---

## Lambda Functions

Ugly syntax: "Captured" variables in brackets, then arguments
in parentheses, then code in braces

Still useful enough to be *encouraged* in many cases:

- Defining a new function object in the only scope where it is used

- Preferring standard generic algorithms (which take function object arguments)

- Improving efficiency over using function pointers for function object arguments

```cpp
// List captured variables in the capture list explicitly where possible.
std::for_each(container.begin(), container.end(), [= local_var](Foo & foo) {
  foo.item = local_var.item;
  foo.item2 = local_var.item2;
});
```

!---

## Modern C++ Guidelines

Code should be as safe and as expressive as possible:

- Mark all overridden `virtual` methods as `override`
- Do not use `new` or `malloc` where it can be avoided
- Use smart pointers, with `std::make_shared<T>()` or `std::make_unique<T>()`
- Prefer `<algorithm>` functions over hand-written loops
- Prefer range `for` except when an index variable is explicitly needed
- Make methods const-correct, avoid `const_cast` where possible

Optimization should be done when critical-path or non-invasive:

- Use `final` keyword on classes and virtual functions with no further
  subclass overrides
- Use `std::move()` when applicable

!---

## Variable Initialization

When creating a new variable use these patterns:

```cpp
unsigned int i = 4;                                   // Built-in types
SomeObject junk(17);                                  // Objects
SomeObject * stuff = functionReturningPointer();      // Pointers
auto & [key, value] = functionReturningKVPair();      // Individual tuple members
auto heap_memory = std::make_unique<HeapObject>(a,b); // Non-container dynamic allocation
std::vector<int> v = {1, 2, 4, 8, 16};                // Simple containers
```

!---

## Trailing Whitespace and Tabs

MOOSE does not allow any trailing whitespace or tabs in the repository. Try running the following
one-liner from the appropriate directory:

```bash
find . -name '*.[Chi]' -or -name '*.py' | xargs perl -pli -e 's/\s+$//'
```

!---

## Includes

Firstly, only include things that are absolutely necessary in header files.  Please use forward
declarations when you can:

```cpp
// Forward declarations
class Something;
```

All non-system includes should use quotes and there is a space between `include` and the filename.

```cpp
#include "LocalInclude.h"
#include "libmesh/libmesh_include.h"

#include <system_library>
```

!---

## In-Code Documentation

Try to document as much as possible, using Doxygen style comments

```cpp
/**
 * The Kernel class is responsible for calculating the residuals for various physics.
 */
class Kernel
{
public:
  /**
   * This constructor should be used most often.  It initializes all internal
   * references needed for residual computation.
   *
   * @param system The system this variable is in
   * @param var_name The variable this Kernel is going to compute a residual for.
   */
  Kernel(System * system, std::string var_name);

  /**
   * This function is used to get stuff based on junk.
   *
   * @param junk The index of the stuff you want to get
   * @return The stuff associated with the junk you passed in
   */
  int returnStuff(int junk);

protected:
  /// This is the stuff this class holds on to.
  std::vector<int> stuff;
};
```

!---

## Python

Where possible, follow the above rules for Python. The only modifications are:

1. Four spaces are used for indenting and
2. Member variables should be named as follows:

```python
class MyClass:
    def __init__(self):
        self.public_member
        self._protected_member
        self.__private_member
```

!---

# Code Recommendations

1. Use references whenever possible
1. Functions should return or accept pointers only if "null" is a
   valid value, in which case it should be tested for on every use
1. When creating a new class include dependent header files in the *.C file whenever possible
1. Avoid using global variables
1. Every class with any virtual functions should have a virtual
   destructor
1. Function definitions should be in *.C files, unless so small that they should definitely be inlined
1. Add assertions when making any assumption that could be violated
   by invalid code
1. Add error handling whenever an assumption could be violated by
   invalid user input
