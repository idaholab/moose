# C++\\
Scope, Memory, and Overloading

!---

## Scope

A scope is the extent of the program where a variable can be seen and used.

- local variables have scope from the point of declaration to the end of the enclosing block { }
- global variables are not enclosed within any scope and are available within the entire file

Variables have a limited lifetime

- When a variable goes out of scope, its destructor is called

Dynamically-allocated (via new) memory *is not* automatically freed at the end of scope

!---

## "Named" Scopes

class scope

```cpp
class MyObject
{
public:
  void myMethod();
};
```

namespace scope

```cpp
namespace MyNamespace
{
  float a;
  void myMethod();
}
```

!---

## Scope Resolution Operator

"double colon" :: is used to refer to members inside of a named scope

```cpp
// definition of the "myMethod" function of "MyObject"
void MyObject::myMethod()
{
  std::cout << "Hello, World!\n";
}
MyNamespace::a = 2.718;
MyNamespace::myMethod();
```

Namespaces permit data organization, but do not have all the features needed for full encapsulation

!---

## Assignment

### (Prequel to Pointers and Refs)

Recall that assignment in C++ uses the "single equals" operator:

```cpp
a = b; // Assignment
```

Assignments are one of the most common operations in programming

Two operands are required

- An assignable location on the left hand side (memory location)
- An expression on the right hand side

!---

## Pointers

Native type just like an int or long

Hold the location of another variable or object in memory

Useful in avoiding expensive copies of large objects

Facilitate shared memory

- Example: One object "owns" the memory associated with some data, and allows others objects access
  through a pointer

!---

## Pointer Syntax

Declare a pointer

```cpp
int *p;
```

Use the +address-of+ operator to initialize a pointer

```cpp
int a;
p = &a;
```

Use the +dereference+ operator to get or set values pointed-to by the pointer

```cpp
*p = 5;                  // set value of "a" through "p"
std::cout << *p << "\n"; // prints 5
std::cout <<  a << "\n"; // prints 5
```

!---

## Pointer Syntax (continued)

```cpp
int a = 5;
int *p;       // declare a pointer
p = &a;       // set 'p' equal to address of 'a'
*p = *p + 2;  // get value pointed to by 'p', add 2,
              // store result in same location
std::cout <<  a << "\n";  // prints 7
std::cout << *p << "\n";  // prints 7
std::cout <<  p << "\n";  // prints an address (0x7fff5fbfe95c)
```

!---

## Pointers are Powerful but Unsafe

On the previous slide we had this:

```cpp
p = &a;
```

But we can do almost anything we want with p!

```cpp
p = p + 1000;
```

Now what happens when we do this?

```cpp
*p;    // Access memory at &a + 1000
```

!---

## References to the Rescue

A reference is an alternative name for an object (Stroustrup), think of it as an alias for the
original variable

```cpp
int a = 5;
int &r = a;  // define and initialize a ref
r = r + 2;
std::cout <<  a << "\n";  // prints 7
std::cout <<  r << "\n";  // prints 7
std::cout << &r << "\n";  // prints address of a
```

!---

## References are Safe

References cannot be modified

```cpp
&r = &r + 1;   // won't compile
```

References never start out un-initialized

```cpp
int &r;     // won't compile
```

- Note, that class declarations may contain references
- If so, initialization must occur in the constructor!

!---

## Summary: Pointers and References

A pointer is a variable that holds a memory address to another variable

```cpp
int *iPtr;  // Declaration
iPtr = &c;
int a = b + *iPtr;
```

A reference is an alternative name for an object (Stroustrup), so it must reference an existing object

```cpp
int &iRef = c;    // Must initialize
int a = b + iRef;
```

!---

## Calling Conventions

What happens when you make a function call?

```cpp
result = someFunction(a, b, my_shape);
```

- If the function changes the values inside of a, b or myshape, are those changes reflected in my
  code?
- Is this call expensive? (Are arguments copied around?)
- C++ by default is "Pass by Value" (copy) but you can pass arguments by reference (alias) with
  additional syntax

!---

## Swap Example (Pass by Value)

```cpp
void swap(int a, int b)
{
  int temp = a;
  a = b;
  b = temp;
}
int i = 1;
int j = 2;
swap (i, j);                  // i and j are arguments
std::cout << i << " " << j;   // prints 1 2
                              // i and j are not swapped
```

!---

## Swap Example (Pass by Reference)

```cpp
void swap(int &a, int &b)
{
  int temp = a;
  a = b;
  b = temp;
}
int i = 1;
int j = 2;
swap (i, j);                  // i and j are arguments
std::cout << i << " " << j;   // prints 2 1
                              // i and j are properly swapped
```

!---

## Dynamic Memory Allocation

Why do we need dynamic memory allocation?

- Data size specified at run time (rather than compile time)
- Persistence without global variables (scopes)
- Efficient use of space
- Flexibility

!---

## Dynamic Memory in C++

"new" allocates memory

"delete" frees memory

Recall that variables typically have limited lifetimes (within the nearest enclosing scope)

Dynamic memory allocations do not have limited lifetimes

- No automatic memory cleanup!
- Watch out for memory leaks
- Should have a "delete" for every "new".

During normal usage, dynamic memory allocation is unnecessary.

!---

## Example: Dynamic Memory

```cpp
int a;
int *b;
b = new int; // dynamic allocation, what is b's value?
a = 4;
*b = 5;
int c = a + *b;
std::cout << c;  // prints 9
delete b;
```

!---

## Example: Dynamic Memory Using References

```cpp
int a;
int *b = new int;    // dynamic allocation
int &r = *b;         // creating a reference to newly created variable
a = 4;
r = 5;
int c = a + r;
std::cout << c;  // prints 9
delete b;
```

!---

## Const

The `const` keyword is used to mark a variable, parameter, method or other argument as constant

Typically used with references and pointers to share objects but guarantee that they will not be
modified

```cpp
{
  std::string name("myObject");
  print(name);
  ...
}
void print(const std::string & name)
{
  // Attempting to modify name here will
  // cause a compile time error
  ...
}
```

!---

## Function Overloading

In C++ you may reuse function names as long as they have different parameter lists or types. A
difference only in the return type is not enough to differentiate overloaded signatures.

```cpp
int foo(int value);
int foo(float value);
int foo(float value, bool is_initialized);
...
```

This is very useful when we get to object "constructors".
