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
  Moose::Kokkos::DispatcherRegistry::addDispatcher<KokkosDispatcherTestObject::TestLoop,
                                                   KokkosDispatcherTestObject>(
      "KokkosDispatcherTestObject");

  Moose::Kokkos::DispatcherRegistry::addReducer<KokkosReducerTestObject::TestLoop,
                                                KokkosReducerTestObject>("KokkosReducerTestObject");
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

TEST_F(KokkosDispatcherTest, reduceKokkosObject)
{
  Kokkos::View<Real *, Kokkos::HostSpace> array("array", 3);
  Moose::Kokkos::Policy policy(0, 3);
  KokkosReducerTestObject object(3);

  auto dispatcher = Moose::Kokkos::DispatcherRegistry::build<KokkosReducerTestObject::TestLoop>(
      &object, "KokkosReducerTestObject");
  dispatcher->parallelReduce(policy, array);

  EXPECT_EQ(array(0), 3);
  EXPECT_EQ(array(1), 6);
  EXPECT_EQ(array(2), 9);
}
