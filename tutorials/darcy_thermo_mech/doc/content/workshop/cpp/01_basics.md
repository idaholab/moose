# C++\\
Fundamentals

!---

## Data Types

| Intrinsic Type | Variant(s) |
| :- | :- |
| bool | |
| char | unsigned |
| int | unsigned, long, short |
| float | |
| double | long |
| void | |

Note, `void` is the "anti-datatype", used in functions returning nothing

Objects of these types can be combined into more complicated
"structure" or "class" objects, or aggregated into arrays or various
"container" classes.

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

Used to group statements together and to define the scope of a function

Creates new layer of scope

```cpp
int x = 2;

{
  int x = 5; // "Shadows" the other x - bad practice
  assert(x == 5);
}

assert(x == 2);
```

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

Scope resolution:

```cpp
a = std::pow(r, 2);     // Calls the standard pow() function
b = YourLib::pow(r, 2); // Calls pow() from YourLib namespace or class

using std::pow;      // Now "pow" can mean "std::pow" automatically
using YourLib::pow;  // Or it can mean "YourLib::pow"...

c = pow(r, 2); // Ambiguous, or deduced from the type of r
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
int i = huge_value;                   // value silently changed!
int j = static_cast<int>(huge_value); // won't help!
```

And consider safer MOOSE tools

```
int i = cast_int<int>(huge_value);    // assertion failure in non-optimized runs
```

!---

## Control Statements

For, While, and Do-While Loops:

```cpp
for (int i=0; i<10; ++i) { foo(i); }
for (auto val : my_container) { foo(val); }
while (boolean-expression)  { bar(); }
do { baz(); } while (boolean-expression);
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

## Declarations and Definitions

In C++ we split our code into multiple files

- headers (*.h)
- bodies (*.C)

Headers generally contain +declarations+

- Statement of the types we will use
- Gives names to types
- The argument and return type signatures of functions

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

(Pointers to) functions *themselves* are also objects, with ugly syntax

```cpp
  returnType (*f_ptr)(type1, type2) = &functionName;
  returnType r = (*f_ptr)(a1, a2);
  do_something_else_with(f_ptr);
```

!---

### Definition Examples

Function definition:

```cpp
returnType functionName(type1 name1, type2 name2)
{
  // statements
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

## Make

A Makefile is a list of dependencies with rules to satisfy those dependencies. All MOOSE-based
applications are supplied with a complete Makefile.

To build a MOOSE-based application, just type:

```text
make
```
