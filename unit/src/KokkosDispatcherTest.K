//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosDispatcherTest.h"

void
KokkosDispatcherTest::SetUp()
{
  Moose::Kokkos::DispatcherRegistry::add<KokkosDispatcherTestObject::TestLoop,
                                         KokkosDispatcherTestObject>("KokkosDispatcherTestObject");
}

TEST_F(KokkosDispatcherTest, dispatchKokkosObject)
{
  Moose::Kokkos::Array<unsigned int> array(3);
  Moose::Kokkos::Policy policy(0, 3);
  KokkosDispatcherTestObject object(array);

  auto dispatcher = Moose::Kokkos::DispatcherRegistry::build<KokkosDispatcherTestObject::TestLoop>(
      &object, "KokkosDispatcherTestObject");
  dispatcher->parallelFor(policy);

  array.copyToHost();

  EXPECT_EQ(array[0], 0);
  EXPECT_EQ(array[1], 1);
  EXPECT_EQ(array[2], 2);
}
