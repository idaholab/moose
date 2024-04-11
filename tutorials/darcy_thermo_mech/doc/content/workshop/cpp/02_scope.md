# C++\\
Scope, Memory, and Overloading

!---

## Scope

A scope is the extent of the program where a variable can be seen and used.

- local variables have scope from the point of declaration to the end of the enclosing block { }
- global variables are not enclosed within any scope and are available within the entire file

Variables have a limited lifetime

- When a variable goes out of scope, its destructor is called

Manually dynamically-allocated (via `new`) memory *is not*
automatically freed at the end of scope, but smart-pointers and
containers will free dynamically-allocated memory in their
destructors.

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
MyObject::myMethod();
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

- An assignable "lvalue" on the left hand side, referring to some
  object
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

The +address-of+ operator on a variable gives a pointer to it, for initializing another pointer

```cpp
int a;
p = &a;
```

The +dereference+ operator on a pointer gives a reference to what it points to, to get or set values

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

A reference is an alternative name for an object (Stroustrup), like an alias for the
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

## References are Safer

References cannot be reseated, nor left un-initialized - even classes
with references must initialize them in the constructor!

```cpp
&r = &r + 1;   // won't compile
int &r;        // won't compile
```

But references can still be incorrectly left "dangling"

```cpp
std::vector<int> four_ints(4);
int &r = four_ints[0];
r = 5;  // Valid: four_ints is now {5,0,0,0}
four_ints.clear();  // four_ints is now {}
r = 6;  // Undefined behavior, nasal demons
```

!---

## Rvalue References Can Be More Efficient

An "lvalue" reference starts with `&`; an "rvalue" reference starts
with `&&`.

Mnemonic: lvalues can usually be assigned to, +L+eft of an = sign;
rvalues can't, so they're found +R+ight of an = sign.

Lvalue code like "copy assignment" is correct and sufficient

```cpp
Foo & Foo::operator= (const Foo & other) {
  this->member1 = other.member1;  // Has to copy everything
  this->member2 = other.member2;
}
```

But rvalue code like "move assignment" may be written for optimization

```cpp
Foo & Foo::operator= (Foo && other) {
  // std::move "makes an lvalue into an rvalue"
  this->member1 = std::move(other.member1);  // Can cheaply "steal" memory
  this->member2 = std::move(other.member2);
}
```

!---

## Summary: Pointers and References

A pointer is a variable that holds a potentially-changeable memory address to another variable

```cpp
int *iPtr;  // Declaration
iPtr = &c;
int a = b + *iPtr;
```

An lvalue reference is an alternative name for an object, and must reference a fixed object

```cpp
int &iRef = c;    // Must initialize
int a = b + iRef;
```

An rvalue reference is an alternative name for a temporary object

```cpp
std::sqrt(a + b);  // "a+b" creates an object which will stop existing shortly
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

## "Swap" Example - Pass by Value

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

## Swap Example - Pass by (Lvalue) Reference

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

"new" allocates memory, "delete" frees it

Recall that variables typically have limited lifetimes (within the nearest enclosing scope)

Dynamic memory allocations do not have limited lifetimes

- No deallocation when a pointer goes out of scope!
- No automatic garbage collection when dynamic memory becomes unreachable!
- Watch out for memory leaks - a missing "delete" is memory that
  is lost until program exit

Modern C++ provides classes that encapsulate allocation, with
destructors that deallocate.

Almost every "new"/"delete" should be a "smart pointer" or container class instead!

!---

## Example: Dynamic Memory

```cpp
  {
    int a = 4;
    int *b = new int; // dynamic allocation; what is b's value?
    auto c = std::make_unique<int>(6); // dynamic allocation
    *b = 5;
    int d = a + *b + *c;
    std::cout << d;  // prints 15
  }
  // a, b, and c are no longer on the stack
  // *c was deleted from the heap by std::unique_ptr
  // *b is leaked memory - we forgot "delete b" and now it's too late!
```

!---

## Example: Dynamic Memory Using References

```cpp
int a;
auto b = std::make_unique<int>(); // dynamic allocation
int &r = *b;     // creating a reference to newly allocated object
a = 4;
r = 5;
int c = a + r;
std::cout << c;  // prints 9
```

!---

## Const

The `const` keyword is used to mark a variable, parameter, method or other argument as constant

Often used with references and pointers to share objects which should not be modified

```cpp
{
  std::string name("myObject");
  print(name);
}
void print(const std::string & name)
{
  name = "MineNow"; // Compile-time error
  const_cast<std::string &>(name) = "MineNow"; // Just bad code
  ...
}
```

!---

## Constexpr

The `constexpr` keyword marks a variable or function as evaluable at compile time

```cpp
constexpr int factorial(int n)
{
  if (n <= 1)
    return 1;
  return n * factorial(n-1);
}
{
  constexpr int a = factorial(6); // Compiles straight to a = 720
  int b = 6;
  function_which_might_modify(b);
  int c = factorial(b); // Computed at run time
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
