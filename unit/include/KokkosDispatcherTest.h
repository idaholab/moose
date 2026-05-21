//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosDispatcher.h"
#include "KokkosArray.h"
#include "KokkosThread.h"

#include "gtest_include.h"

#include <map>
#include <string>

class KokkosDispatcherTestObject
{
public:
  KokkosDispatcherTestObject(Moose::Kokkos::Array<unsigned int> array) : _array(array) {}

  struct TestLoop
  {
  };

  KOKKOS_FUNCTION void
  operator()(TestLoop, const Moose::Kokkos::ThreadID tid, const KokkosDispatcherTestObject &) const
  {
    _array[tid] = tid;
  }

private:
  Moose::Kokkos::Array<unsigned int> _array;
};

class KokkosReducerTestObject
{
public:
  KokkosReducerTestObject(const unsigned int n) : _n(n) {}

  struct TestLoop
  {
  };

  KOKKOS_FUNCTION void operator()(TestLoop,
                                  const Moose::Kokkos::ThreadID tid,
                                  const KokkosReducerTestObject &,
                                  Real * result) const
  {
    for (unsigned int i = 0; i < _n; ++i)
      result[i] += (i + 1) * tid;
  }

  template <typename Derived>
  KOKKOS_FUNCTION void join(Real * result, const Real * source) const
  {
    for (unsigned int i = 0; i < _n; ++i)
      result[i] += source[i];
  }

  template <typename Derived>
  KOKKOS_FUNCTION void init(Real * result) const
  {
    for (unsigned int i = 0; i < _n; ++i)
      result[i] = 0;
  }

private:
  const unsigned int _n;
};

class KokkosDispatcherTest : public ::testing::Test
{
public:
  virtual void SetUp() override;
};
