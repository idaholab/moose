//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFunctorTest.h"

void
KokkosFunctorTest::SetUp()
{
  Moose::Kokkos::FunctorRegistry::add<KokkosFunctorObject>("KokkosFunctorObject");
}

TEST_F(KokkosFunctorTest, callKokkosFunctor)
{
  Moose::Kokkos::Array<char> array(5);
  KokkosFunctorTestObject object(array);

  Kokkos::parallel_for(Kokkos::RangePolicy<ExecSpace>(0, 1), object);

  array.copyToHost();

  EXPECT_EQ(array[0], 'h');
  EXPECT_EQ(array[1], 'e');
  EXPECT_EQ(array[2], 'l');
  EXPECT_EQ(array[3], 'l');
  EXPECT_EQ(array[4], 'o');
}
