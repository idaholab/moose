//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosFunctorRegistry.h"
#include "KokkosFunctor.h"
#include "KokkosArray.h"

#include "gtest_include.h"

class KokkosFunctorObject
{
public:
  KokkosFunctorObject(Moose::Kokkos::Array<char> array) : _array(array) {}

  KOKKOS_FUNCTION void hello() const
  {
    _array[0] = 'h';
    _array[1] = 'e';
    _array[2] = 'l';
    _array[3] = 'l';
    _array[4] = 'o';
  }

private:
  Moose::Kokkos::Array<char> _array;
};

class KokkosFunctorTestObject
{
public:
  KokkosFunctorTestObject(Moose::Kokkos::Array<char> array)
    : _object(array),
      _functor(Moose::Kokkos::FunctorRegistry::build(&_object, "KokkosFunctorObject"))
  {
  }

  KOKKOS_FUNCTION void operator()(const int) const { _functor.hello(); }

private:
  KokkosFunctorObject _object;
  Moose::Kokkos::Functor _functor;
};

class KokkosFunctorTest : public ::testing::Test
{
public:
  virtual void SetUp() override;
};
