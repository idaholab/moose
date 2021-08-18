# C++\\
Fundamentals

!---

## C Preprocessor Commands

`#` Should be the first character on the line

- `#include <iostream>`
- `#include "myheader.h"`
- `#define SOMEWORD value`
- `#ifdef, #ifndef, #endif`

!---

```cpp
#pragma once
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic pop
```

!---

## Intrinsic Data Types

| Basic Type | Variant(s) |
| :- | :- |
|bool | |
|char | unsigned |
| int | unsigned, long, short |
| float | |
| double | long |
| void | |

Note, `void` is the "anti-datatype", used in functions returning nothing

!---

## Operators

| Purpose | Symbols |
| -: | :- |
| Math | `+ - * / % += -= /= %= ++ --` |
| Comparison | `< > <= >= != ==` |
| Logical Comparison | `&& || !` |
| Memory | `* & new delete sizeof` |
| Assignment | `=` |
| Member Access | `-> .` |
| Name Resolution | `::` |


!---

## Curly Braces `{ }`


Used to group statements together

Creates new layer of scope (we will get to this)

!---

## Expressions

Composite mathematical expressions:

```cpp
a = b * (c - 4) / d++;
```

Composite boolean expressions:

```cpp
if (a && b && f()) { e = a; }
```

Note, Operators && and || use "short-circuiting," so "b" and "f()" in the example above may not get
evaluated.

!---

Scope resolution operator:

```cpp
t = std::pow(r, 2);
b = std::sqrt(d);
```

Dot and Pointer Operator:

```cpp
t = my_obj.someFunction();
b = my_ptr->someFunction();
```

!---

## Type Casting

```cpp
float pi = 3.14;
```

```cpp
int approx_pi = static_cast<int>(pi);
```

!---

## Limits to Type Casting

Does not work to change to fundamentally different types

```cpp
float f = (float) "3.14";   // won't compile
```

Be careful with your assumptions

```cpp
unsigned int huge_value = 4294967295; // ok
int i = static_cast<int>(huge_value); // won't work!
```

!---

## Control Statements

For, While, and Do-While Loops:

```cpp
for (int i=0; i<10; ++i) { }
while (boolean-expression)  { }
do { } while (boolean-expression);
```

If-Then-Else Tests:

```cpp
if (boolean-expression) { }
else if (boolean-expression) { }
else { }
```

In the previous examples, +boolean-expression+ is any valid C++ statement which results in true or
false, such as:

- `if (0) // Always false`
- `while (a > 5)`

!---

### Switch Statement

```cpp
switch (expression)
{
case constant1:
  // commands to execute if
  // expression==constant1 ...
  break;
case constant2:
case constant3:
  // commands to execute if
  // expression==constant2 OR expression==constant3...
  break;
default:
  // commands to execute if no previous case matched
}
```

!---

## Declarations and Definitions

In C++ we split our code into multiple files

- headers (*.h)
- bodies (*.C)

Headers generally contain +declarations+

- Statement of the types we will use
- Gives names to types

Bodies generally contain +definitions+

- Our descriptions of those types, including what they do or how they are built
- Memory consumed
- The operations functions perform

!---

### Declaration Examples

Free functions:

```cpp
returnType functionName(type1 name1, type2 name2);
```

Object member functions (methods):

```cpp
class ClassName
{
  returnType methodName(type1 name1, type2 name2);
};
```

!---

### Definition Examples

Function definition:

```cpp
returnType functionName(type1 name1, type2 name2)
{
  // statesments
}
```

Class method definition:

```cpp
returnType ClassName::methodName(type1 name1, type2 name2)
{
   // statements
}
```

!---

### Function Example: Addition

```cpp
#include <iostream>
int addition (int a, int b)
{
  return a + b;
}

int main ()
{
  int z = addition(5,3);
  std::cout << "The result is " << z << "\n";
  return 0;
}
```

!---

### Forward Declaration

```cpp
#include <iostream>
int addition (int a, int b);

int main ()
{
  int z = addition (5,3);
  std::cout << "The result is " << z << "\n";
  return 0;
}

int addition (int a, int b)
{
  return a + b;
}
```

!---

## Make

A Makefile is a list of dependencies with rules to satisfy those dependencies
All MOOSE-based applications are supplied with a complete Makefile
To build a MOOSE-based application just type:

```text
make
```

!---

## Compiling, Linking, Executing

Compile and Link

```
g++ -O3 -o myExample myExample.C
```

Compile only

```
g++ -O3 -o myExample.o -c myExample.C
```

Link only

```
g++ -O3 -o myExample myExample.o
```

!---

## Compiler/Linker Flags

Libraries (`-L`) and Include (`-I`) path
Library Names (`-l`)

- Remove the leading "lib" and trailing file extension when linking
- libutils.so would link as -lutils

```
g++ -I/home/permcj/include -L/home/permcj/lib -lutils -Wall -o myExec myExec.o
```

!---

## Execution

Basic execution

```
./myExec
```

Finding shared libraries at runtime

- Linux: `ldd` and `$LD_LIBRARY_PATH`
- MacOS: `otool`

!---

## Addition Example (continued)

### Header File (add.h)

```cpp
#pragma once
int addition (int a, int b); // Function declaration
```

Headers typically contain declarations only

!---

### Source File (add.C)

```cpp
#include "add.h"
int addition (int a, int b)
{
  return a + b;
}
```

!---

### Driver Program (main.C)

```cpp
#include "add.h"
#include <iostream>
int main ()
{
  int z = addition(5,3);
  std::cout << "The result is " << z;
  return 0;
}
```

!---

## Compiling the Addition Example

1. `g++ -g -c -o add.o add.C`
1. `g++ -g -c -o main.o main.C`
1. `g++ -g -o main main.o add.o`

   - The -c flag means compile only, do not link
   - These commands can be stored in a Makefile and executed automatically with the make command
