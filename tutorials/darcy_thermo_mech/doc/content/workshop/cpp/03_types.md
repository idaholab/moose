# C++\\
Types, Templates, and\\
Standard Template Library (STL)

!---

## Static vs Dynamic Type Systems

C++ is a "statically-typed" language

This means that "type checking" is performed during compile-time as opposed to run-time

Python and MATLAB are examples of "dynamically-typed" languages

!---

## Static Typing Pros and Cons

Pros:

- Safety: compilers can detect many errors
- Optimization: compilers can optimize for size and speed
- Documentation: flow of types and their uses in expression is self documenting

Cons:

- More explicit code is needed to convert ("cast") between types
- Abstracting or creating generic algorithms is more difficult

!---

## Templates

C++ implements the generic programming paradigm with "templates".

Many of the finer details of C++ template usage are beyond the scope of this short tutorial.

Fortunately, only a small amount of syntactic knowledge is required to make effective basic use of
templates.

```cpp
template <class T>
T getMax (T a, T b)
{
  if (a > b)
    return a;
  else
    return b;
}
```

!---

## Templates (continued)

```cpp
template <class T>
T getMax (T a, T b)
{
  return (a > b ? a : b); // "ternary" operator
}
int i = 5, j = 6, k;
float x = 3.142; y = 2.718, z;
k = getMax(i, j);       // uses int version
z = getMax(x, y);       // uses float version
k = getMax<int>(i, j);  // explicitly calls int version
```

!---

## Template Specialization

```cpp
template<class T>
void print(T value)
{
  std::cout << value << std::endl;
}

template<>
void print<bool>(bool value)
{
  if (value)
    std::cout << "true\n";
  else
    std::cout << "false\n";
}
```

!---

## Template Specialization (continued)

```cpp
int main()
{
  int a = 5;
  bool b = true;
  print(a); // prints 5
  print(b); // prints true
}
```

!---

## C++ Standard Template Library (STL)

- [vector](http://www.cplusplus.com/reference/vector/vector/)
- [list](http://www.cplusplus.com/reference/list/list/)
- [map](http://www.cplusplus.com/reference/map/map/), [multimap](http://www.cplusplus.com/reference/multimap/multimap)
- [set](http://www.cplusplus.com/reference/set/set/), [multiset](http://www.cplusplus.com/reference/set/set)
- [stack](http://www.cplusplus.com/reference/stack/stack/)
- [queue](http://www.cplusplus.com/reference/queue/queue/), [priority_queue](http://www.cplusplus.com/reference/priorityqueue/priorityqueue)
- [deque](http://www.cplusplus.com/reference/deque/deque/)
- [bitset](http://www.cplusplus.com/reference/bitset/bitset/)
- [unordered_map](http://www.cplusplus.com/reference/unorderedmap/unorderedmap/)
- [unordered_set](http://www.cplusplus.com/reference/unorderedset/unorderedset/)

!---

## Using the C++ Vector Container

```cpp
#include <vector>
int main()
{
  // start with 10 elements
  std::vector<int> v(10);
  for (unsigned int i=0; i<v.size(); ++i)
    v[i] = i;
}
```

!---

```cpp
#include <vector>
int main()
{
  // start with 0 elements
  std::vector<int> v;
  for (unsigned int i=0; i<10; ++i)
    v.push_back(i);
}
```

!---

```cpp
#include <vector>
int main()
{
  // start with 0 elements
  std::vector<int> v;
  v.resize(10);  // creates 10 elements
  for (unsigned int i=0; i<10; ++i)
    v[i] = i;
}
```
