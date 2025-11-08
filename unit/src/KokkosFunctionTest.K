//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunctionTest.h"

void
KokkosFunctionTest::SetUp()
{
  Moose::Kokkos::FunctorRegistry::addFunction<KokkosDummyFunction>("KokkosDummyFunction");
}

TEST_F(KokkosFunctionTest, virtualFunctionCall)
{
  Moose::Kokkos::Array<int> array(3);
  KokkosDummyFunction dummy_function;
  Moose::Kokkos::Function function(
      Moose::Kokkos::FunctorRegistry::buildFunction(&dummy_function, "KokkosDummyFunction"));
  KokkosFunctionTestObject object(array, function);

  Kokkos::parallel_for(3, object);
  Kokkos::fence();

  array.copyToHost();

  EXPECT_EQ(array[0], 0);
  EXPECT_EQ(array[1], 1);
  EXPECT_EQ(array[2], 2);
}
