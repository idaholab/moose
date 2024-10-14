# C++\\
Fundamentals

!---

## Intrinsic Data Types

| Basic Type | Variant(s) |
| :- | :- |
| bool | |
| char | unsigned |
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

Used to group statements together and to define the scope of a function

Creates new layer of scope

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

A Makefile is a list of dependencies with rules to satisfy those dependencies
All MOOSE-based applications are supplied with a complete Makefile
To build a MOOSE-based application just type:

```text
make
```
