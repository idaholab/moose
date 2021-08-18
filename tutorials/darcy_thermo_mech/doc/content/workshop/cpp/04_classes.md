# C++\\
Classes and Object Oriented Programming

!---

## Object Oriented Definitions

A "class" is a new data type that contains data and methods for operating on that data

- Think of it as a "blue print" for building an object

An "interface" is defined as a class's publicly available "methods" and "members"

An "instance" is a variable of one of these new data types.

- Also known as an "object"
- Analogy: You can use one "blue-print" to build many buildings. You can use one "class" to build
  many "objects".

!---

## Object Oriented Design

Instead of manipulating data, one manipulates objects that have defined interfaces

Data encapsulation is the idea that objects or new types should be black boxes. Implementation
details are unimportant as long as an object works as advertised without side effects.

Inheritance gives us the ability to abstract or "factor out" common data and functions out of related
types into a single location for consistency (avoids code duplication) and enables *code re-use*.

Polymorphism gives us the ability to write *generic algorithms* that automatically work with derived
types.

!---

## Encapsulation (Point.h)

```cpp
class Point
{
public:
  Point(float x, float y);   // Constructor

  // Accessors
  float getX();
  float getY();
  void setX(float x);
  void setY(float y);
private:
  float _x, _y;
};
```

!---

## Constructors

The method that is called explicitly or implicitly to build an object

Always has the same name as the class with no return type

May have many overloaded versions with different parameters

The constructor body uses a special syntax for initialization called an initialization list

Every member that can be initialized in the initialized list - should be

- References have to be initialized here

!---

# Point Class Definitions (Point.C)

```cpp
#include "Point.h"

Point::Point(float x, float y): _x(x), _y(y) { }
float Point::getX() { return _x; }
float Point::getY() { return _y; }
void Point::setX(float x) { _x = x; }
void Point::setY(float y) { _y = y; }
```

The data is safely encapsulated so we can change the implementation without affecting users of this
type


!---

## Changing the Implementation (Point.h)

```cpp
class Point
{
public:
  Point(float x, float y);
  float getX();
  float getY();
  void setX(float x);
  void setY(float y);
private:
  // Store a vector of values rather than separate scalars
  std::vector<float> _coords;
};
```

!--

## New Point Class Body (Point.C)

```cpp
#include "Point.h"
Point::Point(float x, float y)
{
  _coords.push_back(x);
  _coords.push_back(y);
}

float Point::getX() { return _coords[0]; }
float Point::getY() { return _coords[1]; }
void Point::setX(float x) { _coords[0] = x; }
void Point::setY(float y) { _coords[1] = y; }
```

!---

## Using the Point Class (main.C)

```cpp
#include "Point.h"
int main()
{
  Point p1(1, 2);
  Point p2 = Point(3, 4);
  Point p3; // compile error, no default constructor
  std::cout << p1.getX() << "," << p1.getY() << "\n"
            << p2.getX() << "," << p2.getY() << "\n";
}
```

!---

## Operator Overloading

For some user-defined types (objects) it makes sense to use built-in operators to perform functions
with those types

For example, without operator overloading, adding the coordinates of two points and assigning the
result to a third object might be performed like this:

```cpp
Point a(1,2), b(3,4), c(5,6);
// Assign c = a + b using accessors
c.setX(a.getX() + b.getX());
c.setY(a.getY() + b.getY());
```

However the ability to reuse existing operators on new types makes the following possible:

```cpp
c = a + b;
```

!---

## Operator Overloading (continued)

Inside our Point class, we define new member functions with the special operator keyword:

```cpp
Point Point::operator+(const Point & p)
{
  return Point(_x + p._x, _y + p._y);
}

Point & Point::operator=(const Point & p)
{
  _x = p._x;
  _y = p._y;
  return *this;
}
```

!---

## Using "Point" with Operators

```cpp
#include "Point.h"

int main()
{
  Point p1(0, 0), p2(1, 2), p3(3, 4);
  p1 = p2 + p3;
  std::cout << p1.getX() << "," << p1.getY() << "\n";
}
```

!---

## A More Advanced Example (Shape.h)

```cpp
class Shape {
public:
  Shape(int x=0, int y=0): _x(x), _y(y) {}  // Constructor
  virtual ~Shape() {} // Destructor
  virtual float area()=0;  // Pure Virtual Function
  void printPosition();    // Body appears elsewhere

protected:
  // Coordinates at the centroid of the shape
  int _x;
  int _y;
};
```

!---

## The Derived Class: Rectangle.h

```cpp
#include "Shape.h"
class Rectangle: public Shape
{
public:
  Rectangle(int width, int height, int x=0, int y=0) :
    Shape(x,y),
    _width(width),
    _height(height)
  {}

  virtual ~Rectangle() {}

  virtual float area() { return _width * _height; }

protected:
  int _width;
  int _height;
};
```

!---

# A Derived Class: Circle.h

```cpp
#include "Shape.h"
class Circle: public Shape
{
public:
  Circle(int radius, int x=0, int y=0) :
    Shape(x,y),
    _radius(radius)
  {}

  virtual ~Circle() {}

  virtual float area() { return PI * _radius * _radius; }
protected:
  int _radius;
  const double PI = 3.14159265359;
};
```

!---

## Inheritance (Is a...)

When using inheritance, the derived class can be described in terms of the base class

- A Rectangle "is a" Shape

Derived classes are "type" compatible with the base class (or any of its ancestors)

- We can use a base class variable to point to or refer to an instance of a derived class

```cpp
Rectangle rectangle(3, 4);
Shape & s_ref = rectangle;
Shape * s_ptr = &rectangle;
```

!---

## Deciphering Long Declarations

Read the declaration from right to left

```cpp
// mesh is a pointer to a Mesh object
Mesh * mesh;

// params is a reference to an InputParameters object
InputParameters & params;

// the following are identical
// value is a reference to a constant Real object
const Real & value;
Real const & value;
```

!---

## Writing a Generic Algorithm

```cpp
// create a couple of shapes
Rectangle r(3, 4);
Circle c(3, 10, 10);
printInformation(r);   // pass a Rectangle into a Shape reference
printInformation(c);   // pass a Circle into a Shape reference
...
void printInformation(const Shape & shape)
{
  shape.printPosition();
  std::cout << shape.area() << '\n';
}
// (0, 0)
// 12
// (10, 10)
// 28.274
```

!---

## Homework Ideas

1. Implement a new Shape called Square. Try deriving from Rectangle directly instead of Shape. What
   advantages/disadvantages do the two designs have?
1. Implement a Triangle shape. What interesting subclasses of Triangle can you imagine?
1. Add another constructor to the Rectangle class that accepts coordinates instead of height and
   width.
