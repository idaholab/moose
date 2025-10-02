//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosArrayTest.h"

TEST_F(KokkosArrayTest, jaggedArray11)
{
  Moose::Kokkos::JaggedArray<unsigned int, 1, 1> array(3);

  for (unsigned int I = 0; I < 3; ++I)
    array.reserve({I}, {I + 1});

  array.finalize();

  Kokkos::RangePolicy<> policy(0, 3);
  KokkosJaggedArrayTestObject1D<1> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int i = 0; i < I + 1; ++i)
      EXPECT_EQ(array(I)(i), i);
}

TEST_F(KokkosArrayTest, jaggedArray21)
{
  Moose::Kokkos::JaggedArray<unsigned int, 2, 1> array(3);

  for (unsigned int I = 0; I < 3; ++I)
    array.reserve({I}, {I + 1, I + 1});

  array.finalize();

  Kokkos::RangePolicy<> policy(0, 3);
  KokkosJaggedArrayTestObject1D<2> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int i = 0; i < I + 1; ++i)
      for (unsigned int j = 0; j < I + 1; ++j)
        EXPECT_EQ(array(I)(i, j), i + j);
}

TEST_F(KokkosArrayTest, jaggedArray31)
{
  Moose::Kokkos::JaggedArray<unsigned int, 3, 1> array(3);

  for (unsigned int I = 0; I < 3; ++I)
    array.reserve({I}, {I + 1, I + 1, I + 1});

  array.finalize();

  Kokkos::RangePolicy<> policy(0, 3);
  KokkosJaggedArrayTestObject1D<3> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int i = 0; i < I + 1; ++i)
      for (unsigned int j = 0; j < I + 1; ++j)
        for (unsigned int k = 0; k < I + 1; ++k)
          EXPECT_EQ(array(I)(i, j, k), i + j + k);
}

TEST_F(KokkosArrayTest, jaggedArray12)
{
  Moose::Kokkos::JaggedArray<unsigned int, 1, 2> array(3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      array.reserve({I, J}, {I + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 3});
  KokkosJaggedArrayTestObject2D<1> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int i = 0; i < I + 1; ++i)
        EXPECT_EQ(array(I, J)(i), i);
}

TEST_F(KokkosArrayTest, jaggedArray22)
{
  Moose::Kokkos::JaggedArray<unsigned int, 2, 2> array(3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      array.reserve({I, J}, {I + 1, J + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 3});
  KokkosJaggedArrayTestObject2D<2> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int i = 0; i < I + 1; ++i)
        for (unsigned int j = 0; j < J + 1; ++j)
          EXPECT_EQ(array(I, J)(i, j), i + j);
}

TEST_F(KokkosArrayTest, jaggedArray32)
{
  Moose::Kokkos::JaggedArray<unsigned int, 3, 2> array(3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      array.reserve({I, J}, {I + 1, J + 1, J + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 3});
  KokkosJaggedArrayTestObject2D<3> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int i = 0; i < I + 1; ++i)
        for (unsigned int j = 0; j < J + 1; ++j)
          for (unsigned int k = 0; k < J + 1; ++k)
            EXPECT_EQ(array(I, J)(i, j, k), i + j + k);
}

TEST_F(KokkosArrayTest, jaggedArray13)
{
  Moose::Kokkos::JaggedArray<unsigned int, 1, 3> array(3, 3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        array.reserve({I, J, K}, {I + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 3, 3});
  KokkosJaggedArrayTestObject3D<1> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        for (unsigned int i = 0; i < I + 1; ++i)
          EXPECT_EQ(array(I, J, K)(i), i);
}

TEST_F(KokkosArrayTest, jaggedArray23)
{
  Moose::Kokkos::JaggedArray<unsigned int, 2, 3> array(3, 3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        array.reserve({I, J, K}, {I + 1, J + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 3, 3});
  KokkosJaggedArrayTestObject3D<2> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        for (unsigned int i = 0; i < I + 1; ++i)
          for (unsigned int j = 0; j < J + 1; ++j)
            EXPECT_EQ(array(I, J, K)(i, j), i + j);
}

TEST_F(KokkosArrayTest, jaggedArray33)
{
  Moose::Kokkos::JaggedArray<unsigned int, 3, 3> array(3, 3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        array.reserve({I, J, K}, {I + 1, J + 1, K + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 3, 3});
  KokkosJaggedArrayTestObject3D<3> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        for (unsigned int i = 0; i < I + 1; ++i)
          for (unsigned int j = 0; j < J + 1; ++j)
            for (unsigned int k = 0; k < K + 1; ++k)
              EXPECT_EQ(array(I, J, K)(i, j, k), i + j + k);
}

TEST_F(KokkosArrayTest, jaggedArraySparse)
{
  Moose::Kokkos::JaggedArray<unsigned int, 3, 3> array(3, 3, 3);

  for (unsigned int I = 0; I < 3; I += 2)
    for (unsigned int J = 1; J < 3; J += 2)
      for (unsigned int K = 0; K < 3; K += 2)
        array.reserve({I, J, K}, {I + 1, J + 1, K + 1});

  array.finalize();

  for (unsigned int I = 1; I < 3; I += 2)
    for (unsigned int J = 0; J < 3; J += 2)
      for (unsigned int K = 1; K < 3; K += 2)
      {
        EXPECT_EQ(array(I, J, K).n(0), 0);
        EXPECT_EQ(array(I, J, K).n(1), 0);
        EXPECT_EQ(array(I, J, K).n(2), 0);
        EXPECT_EQ(array(I, J, K).size(), 0);
      }
}
