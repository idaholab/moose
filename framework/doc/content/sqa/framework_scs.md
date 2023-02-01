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
    ... Do something with index i

  // Looping to a container size (where range-for isn't sufficient)
  for (MooseIndex(vector) i = 0; i < vector.size(); ++i)
    ... Do something with index i
```

## Lambdas

List captured variables (by value or reference) in the capture list explicitly where possible.

```C++
  std::for_each(container.begin(), container.end(), [=local_var](Foo & foo) {
    foo.item = local_var;
  });
```

## Other C++ Notes

- Use the `override` keyword on overridden `virtual` methods
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
