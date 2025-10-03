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
  operator()(TestLoop, const ThreadID tid, const KokkosDispatcherTestObject &) const
  {
    _array[tid] = tid;
  }

private:
  Moose::Kokkos::Array<unsigned int> _array;
};

class KokkosDispatcherTest : public ::testing::Test
{
public:
  virtual void SetUp() override;
};
