//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosFunction.h"

#include "gtest_include.h"

using Moose::Kokkos::Real3;

class KokkosDummyFunction
{
public:
  KOKKOS_FUNCTION Real value(Real t, Real3) const { return t; }
  KOKKOS_FUNCTION Real3 vectorValue(Real, Real3) const { return Real3(0); }
  KOKKOS_FUNCTION Real3 gradient(Real, Real3) const { return Real3(0); }
  KOKKOS_FUNCTION Real3 curl(Real, Real3) const { return Real3(0); }
  KOKKOS_FUNCTION Real div(Real, Real3) const { return 0; }
  KOKKOS_FUNCTION Real timeDerivative(Real, Real3) const { return 0; }
  KOKKOS_FUNCTION Real timeIntegral(Real, Real, Real3) const { return 0; }
  KOKKOS_FUNCTION Real integral() const { return 0; }
  KOKKOS_FUNCTION Real average() const { return 0; }
};

class KokkosFunctionTestObject
{
public:
  KokkosFunctionTestObject(Moose::Kokkos::Array<int> array, Moose::Kokkos::Function function)
    : _array(array), _function(function)
  {
  }

  KOKKOS_FUNCTION void operator()(const int i) const { _array[i] = _function.value(i, Real3(0)); }

private:
  Moose::Kokkos::Array<int> _array;
  Moose::Kokkos::Function _function;
};

class KokkosFunctionTest : public ::testing::Test
{
public:
  virtual void SetUp() override;
};
