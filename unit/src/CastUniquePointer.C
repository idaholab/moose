//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "CastUniquePointer.h"

class A
{
public:
  virtual ~A() {}
};

class B : public A
{
public:
  virtual ~B() {}
};

class C : public A
{
public:
  virtual ~C() {}
};

struct ADeleter
{
  void operator()(A * a) { delete a; }
};

TEST(CastUniquePointer, test)
{
  std::unique_ptr<A, ADeleter> b(new B);

  std::unique_ptr<A> c(new C);

  std::unique_ptr<C> c_ptr;

  {
    auto worked = dynamic_pointer_cast<C>(c);
  }

  {
    auto worked = dynamic_pointer_cast<C>(b);
    ASSERT_FALSE((bool)worked);
  }
}
