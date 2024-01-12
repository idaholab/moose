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

Use `auto` for most new code unless it complicates readability. Make sure your variables have
+good+ names when using auto!

```cpp
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

Do not use `auto` in any kind of function or method declaration

!---

## Lambdas

```cpp
// List captured variables (by value or reference) in the capture list explicitly where possible.
std::for_each(container.begin(), container.end(), [= local_var](Foo & foo) {
  foo.item = local_var;
  foo.item2 = local_var2;
});
```

!---

## C++11

- Use the `override` keyword on overridden `virtual` methods
- Use `std::make_shared<T>()` when allocating new memory for shared pointers
- Use `std::make_unique<T>()` when allocating new memory for unique pointers
- Make use of std::move() for efficiency when possible

!---

## Variable Initialization

When creating a new variable use these patterns:

```cpp
unsigned int i = 4;  // Built-in types
SomeObject junk(17); // Objects
SomeObject * stuff = new SomeObject(18); // Pointers
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
1. Methods should return pointers to objects if returned objects are stored as pointers and references
   if returned objects are stored as references
1. When creating a new class include dependent header files in the *.C file whenever possible
1. Avoid using a global variables
1. Every destructor must be virtual
1. All function definitions should be in *.C files, if possible
