//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosArrayTest.h"

TEST_F(KokkosArrayTest, Array1D)
{
  Moose::Kokkos::Array<unsigned int, 1> array(3);

  Kokkos::RangePolicy<> policy(0, 3);
  KokkosArrayTestObject1D object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    EXPECT_EQ(array[i], i);
}

TEST_F(KokkosArrayTest, Array2D)
{
  Moose::Kokkos::Array<unsigned int, 2> array(3, 4);

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 4});
  KokkosArrayTestObject2D object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
    {
      unsigned int idx = i + j * 3;
      EXPECT_EQ(array[idx], i + j);
    }
}

TEST_F(KokkosArrayTest, Array3D)
{
  Moose::Kokkos::Array<unsigned int, 3> array(3, 4, 5);

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 4, 5});
  KokkosArrayTestObject3D object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
      for (unsigned int k = 0; k < 5; ++k)
      {
        unsigned int idx = i + j * 3 + k * 3 * 4;
        EXPECT_EQ(array[idx], i + j + k);
      }
}

TEST_F(KokkosArrayTest, Array4D)
{
  Moose::Kokkos::Array<unsigned int, 4> array(3, 4, 5, 6);

  Kokkos::MDRangePolicy<Kokkos::Rank<4>> policy({0, 0, 0, 0}, {3, 4, 5, 6});
  KokkosArrayTestObject4D object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
      for (unsigned int k = 0; k < 5; ++k)
        for (unsigned int l = 0; l < 6; ++l)
        {
          unsigned int idx = i + j * 3 + k * 3 * 4 + l * 3 * 4 * 5;
          EXPECT_EQ(array[idx], i + j + k + l);
        }
}

TEST_F(KokkosArrayTest, Array5D)
{
  Moose::Kokkos::Array<unsigned int, 5> array(3, 4, 5, 6, 7);

  Kokkos::MDRangePolicy<Kokkos::Rank<5>> policy({0, 0, 0, 0, 0}, {3, 4, 5, 6, 7});
  KokkosArrayTestObject5D object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
      for (unsigned int k = 0; k < 5; ++k)
        for (unsigned int l = 0; l < 6; ++l)
          for (unsigned int m = 0; m < 7; ++m)
          {
            unsigned int idx = i + j * 3 + k * 3 * 4 + l * 3 * 4 * 5 + m * 3 * 4 * 5 * 6;
            EXPECT_EQ(array[idx], i + j + k + l + m);
          }
}

TEST_F(KokkosArrayTest, Array1D4ByteIndex)
{
  Moose::Kokkos::Array<unsigned int, 1, unsigned int> array(3);

  Kokkos::RangePolicy<> policy(0, 3);
  KokkosArrayTestObject1D<unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    EXPECT_EQ(array[i], i);
}

TEST_F(KokkosArrayTest, Array2D4ByteIndex)
{
  Moose::Kokkos::Array<unsigned int, 2, unsigned int> array(3, 4);

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 4});
  KokkosArrayTestObject2D<unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
    {
      unsigned int idx = i + j * 3;
      EXPECT_EQ(array[idx], i + j);
    }
}

TEST_F(KokkosArrayTest, Array3D4ByteIndex)
{
  Moose::Kokkos::Array<unsigned int, 3, unsigned int> array(3, 4, 5);

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 4, 5});
  KokkosArrayTestObject3D<unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
      for (unsigned int k = 0; k < 5; ++k)
      {
        unsigned int idx = i + j * 3 + k * 3 * 4;
        EXPECT_EQ(array[idx], i + j + k);
      }
}

TEST_F(KokkosArrayTest, Array4D4ByteIndex)
{
  Moose::Kokkos::Array<unsigned int, 4, unsigned int> array(3, 4, 5, 6);

  Kokkos::MDRangePolicy<Kokkos::Rank<4>> policy({0, 0, 0, 0}, {3, 4, 5, 6});
  KokkosArrayTestObject4D<unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
      for (unsigned int k = 0; k < 5; ++k)
        for (unsigned int l = 0; l < 6; ++l)
        {
          unsigned int idx = i + j * 3 + k * 3 * 4 + l * 3 * 4 * 5;
          EXPECT_EQ(array[idx], i + j + k + l);
        }
}

TEST_F(KokkosArrayTest, Array5D4ByteIndex)
{
  Moose::Kokkos::Array<unsigned int, 5, unsigned int> array(3, 4, 5, 6, 7);

  Kokkos::MDRangePolicy<Kokkos::Rank<5>> policy({0, 0, 0, 0, 0}, {3, 4, 5, 6, 7});
  KokkosArrayTestObject5D<unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
      for (unsigned int k = 0; k < 5; ++k)
        for (unsigned int l = 0; l < 6; ++l)
          for (unsigned int m = 0; m < 7; ++m)
          {
            unsigned int idx = i + j * 3 + k * 3 * 4 + l * 3 * 4 * 5 + m * 3 * 4 * 5 * 6;
            EXPECT_EQ(array[idx], i + j + k + l + m);
          }
}

TEST_F(KokkosArrayTest, Array2DRightLayout)
{
  Moose::Kokkos::Array<unsigned int, 2, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> array(3, 4);

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 4});
  KokkosArrayTestObject2D<dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
    {
      unsigned int idx = j + i * 4;
      EXPECT_EQ(array[idx], i + j);
    }
}

TEST_F(KokkosArrayTest, Array3DRightLayout)
{
  Moose::Kokkos::Array<unsigned int, 3, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> array(
      3, 4, 5);

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 4, 5});
  KokkosArrayTestObject3D<dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
      for (unsigned int k = 0; k < 5; ++k)
      {
        unsigned int idx = k + j * 5 + i * 5 * 4;
        EXPECT_EQ(array[idx], i + j + k);
      }
}

TEST_F(KokkosArrayTest, Array4DRightLayout)
{
  Moose::Kokkos::Array<unsigned int, 4, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> array(
      3, 4, 5, 6);

  Kokkos::MDRangePolicy<Kokkos::Rank<4>> policy({0, 0, 0, 0}, {3, 4, 5, 6});
  KokkosArrayTestObject4D<dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
      for (unsigned int k = 0; k < 5; ++k)
        for (unsigned int l = 0; l < 6; ++l)
        {
          unsigned int idx = l + k * 6 + j * 6 * 5 + i * 6 * 5 * 4;
          EXPECT_EQ(array[idx], i + j + k + l);
        }
}

TEST_F(KokkosArrayTest, Array5DRightLayout)
{
  Moose::Kokkos::Array<unsigned int, 5, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> array(
      3, 4, 5, 6, 7);

  Kokkos::MDRangePolicy<Kokkos::Rank<5>> policy({0, 0, 0, 0, 0}, {3, 4, 5, 6, 7});
  KokkosArrayTestObject5D<dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 4; ++j)
      for (unsigned int k = 0; k < 5; ++k)
        for (unsigned int l = 0; l < 6; ++l)
          for (unsigned int m = 0; m < 7; ++m)
          {
            unsigned int idx = m + l * 7 + k * 7 * 6 + j * 7 * 6 * 5 + i * 7 * 6 * 5 * 4;
            EXPECT_EQ(array[idx], i + j + k + l + m);
          }
}

TEST_F(KokkosArrayTest, jaggedArray1D1D)
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
      EXPECT_EQ(array(I)[i], i);
}

TEST_F(KokkosArrayTest, jaggedArray2D1D)
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
      {
        unsigned int idx = i + j * (I + 1);
        EXPECT_EQ(array(I)[idx], i + j);
      }
}

TEST_F(KokkosArrayTest, jaggedArray3D1D)
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
        {
          unsigned int idx = i + j * (I + 1) + k * (I + 1) * (I + 1);
          EXPECT_EQ(array(I)[idx], i + j + k);
        }
}

TEST_F(KokkosArrayTest, jaggedArray1D2D)
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
        EXPECT_EQ(array(I, J)[i], i);
}

TEST_F(KokkosArrayTest, jaggedArray2D2D)
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
        {
          unsigned int idx = i + j * (I + 1);
          EXPECT_EQ(array(I, J)[idx], i + j);
        }
}

TEST_F(KokkosArrayTest, jaggedArray3D2D)
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
          {
            unsigned int idx = i + j * (I + 1) + k * (I + 1) * (J + 1);
            EXPECT_EQ(array(I, J)[idx], i + j + k);
          }
}

TEST_F(KokkosArrayTest, jaggedArray1D3D)
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
          EXPECT_EQ(array(I, J, K)[i], i);
}

TEST_F(KokkosArrayTest, jaggedArray2D3D)
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
          {
            unsigned int idx = i + j * (I + 1);
            EXPECT_EQ(array(I, J, K)[idx], i + j);
          }
}

TEST_F(KokkosArrayTest, jaggedArray3D3D)
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
            {
              unsigned int idx = i + j * (I + 1) + k * (I + 1) * (J + 1);
              EXPECT_EQ(array(I, J, K)[idx], i + j + k);
            }
}

TEST_F(KokkosArrayTest, jaggedArray1D1D4ByteIndex)
{
  Moose::Kokkos::JaggedArray<unsigned int, 1, 1, unsigned int> array(3);

  for (unsigned int I = 0; I < 3; ++I)
    array.reserve({I}, {I + 1});

  array.finalize();

  Kokkos::RangePolicy<> policy(0, 3);
  KokkosJaggedArrayTestObject1D<1, unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int i = 0; i < I + 1; ++i)
      EXPECT_EQ(array(I)[i], i);
}

TEST_F(KokkosArrayTest, jaggedArray2D1D4ByteIndex)
{
  Moose::Kokkos::JaggedArray<unsigned int, 2, 1, unsigned int> array(3);

  for (unsigned int I = 0; I < 3; ++I)
    array.reserve({I}, {I + 1, I + 1});

  array.finalize();

  Kokkos::RangePolicy<> policy(0, 3);
  KokkosJaggedArrayTestObject1D<2, unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int i = 0; i < I + 1; ++i)
      for (unsigned int j = 0; j < I + 1; ++j)
      {
        unsigned int idx = i + j * (I + 1);
        EXPECT_EQ(array(I)[idx], i + j);
      }
}

TEST_F(KokkosArrayTest, jaggedArray3D1D4ByteIndex)
{
  Moose::Kokkos::JaggedArray<unsigned int, 3, 1, unsigned int> array(3);

  for (unsigned int I = 0; I < 3; ++I)
    array.reserve({I}, {I + 1, I + 1, I + 1});

  array.finalize();

  Kokkos::RangePolicy<> policy(0, 3);
  KokkosJaggedArrayTestObject1D<3, unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int i = 0; i < I + 1; ++i)
      for (unsigned int j = 0; j < I + 1; ++j)
        for (unsigned int k = 0; k < I + 1; ++k)
        {
          unsigned int idx = i + j * (I + 1) + k * (I + 1) * (I + 1);
          EXPECT_EQ(array(I)[idx], i + j + k);
        }
}

TEST_F(KokkosArrayTest, jaggedArray1D2D4ByteIndex)
{
  Moose::Kokkos::JaggedArray<unsigned int, 1, 2, unsigned int> array(3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      array.reserve({I, J}, {I + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 3});
  KokkosJaggedArrayTestObject2D<1, unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int i = 0; i < I + 1; ++i)
        EXPECT_EQ(array(I, J)[i], i);
}

TEST_F(KokkosArrayTest, jaggedArray2D2D4ByteIndex)
{
  Moose::Kokkos::JaggedArray<unsigned int, 2, 2, unsigned int> array(3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      array.reserve({I, J}, {I + 1, J + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 3});
  KokkosJaggedArrayTestObject2D<2, unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int i = 0; i < I + 1; ++i)
        for (unsigned int j = 0; j < J + 1; ++j)
        {
          unsigned int idx = i + j * (I + 1);
          EXPECT_EQ(array(I, J)[idx], i + j);
        }
}

TEST_F(KokkosArrayTest, jaggedArray3D2D4ByteIndex)
{
  Moose::Kokkos::JaggedArray<unsigned int, 3, 2, unsigned int> array(3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      array.reserve({I, J}, {I + 1, J + 1, J + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 3});
  KokkosJaggedArrayTestObject2D<3, unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int i = 0; i < I + 1; ++i)
        for (unsigned int j = 0; j < J + 1; ++j)
          for (unsigned int k = 0; k < J + 1; ++k)
          {
            unsigned int idx = i + j * (I + 1) + k * (I + 1) * (J + 1);
            EXPECT_EQ(array(I, J)[idx], i + j + k);
          }
}

TEST_F(KokkosArrayTest, jaggedArray1D3D4ByteIndex)
{
  Moose::Kokkos::JaggedArray<unsigned int, 1, 3, unsigned int> array(3, 3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        array.reserve({I, J, K}, {I + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 3, 3});
  KokkosJaggedArrayTestObject3D<1, unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        for (unsigned int i = 0; i < I + 1; ++i)
          EXPECT_EQ(array(I, J, K)[i], i);
}

TEST_F(KokkosArrayTest, jaggedArray2D3D4ByteIndex)
{
  Moose::Kokkos::JaggedArray<unsigned int, 2, 3, unsigned int> array(3, 3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        array.reserve({I, J, K}, {I + 1, J + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 3, 3});
  KokkosJaggedArrayTestObject3D<2, unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        for (unsigned int i = 0; i < I + 1; ++i)
          for (unsigned int j = 0; j < J + 1; ++j)
          {
            unsigned int idx = i + j * (I + 1);
            EXPECT_EQ(array(I, J, K)[idx], i + j);
          }
}

TEST_F(KokkosArrayTest, jaggedArray3D3D4ByteIndex)
{
  Moose::Kokkos::JaggedArray<unsigned int, 3, 3, unsigned int> array(3, 3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        array.reserve({I, J, K}, {I + 1, J + 1, K + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 3, 3});
  KokkosJaggedArrayTestObject3D<3, unsigned int> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        for (unsigned int i = 0; i < I + 1; ++i)
          for (unsigned int j = 0; j < J + 1; ++j)
            for (unsigned int k = 0; k < K + 1; ++k)
            {
              unsigned int idx = i + j * (I + 1) + k * (I + 1) * (J + 1);
              EXPECT_EQ(array(I, J, K)[idx], i + j + k);
            }
}

TEST_F(KokkosArrayTest, jaggedArray1D1DRightLayout)
{
  Moose::Kokkos::JaggedArray<unsigned int, 1, 1, dof_id_type, Moose::Kokkos::LayoutType::RIGHT>
      array(3);

  for (unsigned int I = 0; I < 3; ++I)
    array.reserve({I}, {I + 1});

  array.finalize();

  Kokkos::RangePolicy<> policy(0, 3);
  KokkosJaggedArrayTestObject1D<1, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int i = 0; i < I + 1; ++i)
      EXPECT_EQ(array(I)[i], i);
}

TEST_F(KokkosArrayTest, jaggedArray2D1DRightLayout)
{
  Moose::Kokkos::JaggedArray<unsigned int, 2, 1, dof_id_type, Moose::Kokkos::LayoutType::RIGHT>
      array(3);

  for (unsigned int I = 0; I < 3; ++I)
    array.reserve({I}, {I + 1, I + 1});

  array.finalize();

  Kokkos::RangePolicy<> policy(0, 3);
  KokkosJaggedArrayTestObject1D<2, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int i = 0; i < I + 1; ++i)
      for (unsigned int j = 0; j < I + 1; ++j)
      {
        unsigned int idx = j + i * (I + 1);
        EXPECT_EQ(array(I)[idx], i + j);
      }
}

TEST_F(KokkosArrayTest, jaggedArray3D1DRightLayout)
{
  Moose::Kokkos::JaggedArray<unsigned int, 3, 1, dof_id_type, Moose::Kokkos::LayoutType::RIGHT>
      array(3);

  for (unsigned int I = 0; I < 3; ++I)
    array.reserve({I}, {I + 1, I + 1, I + 1});

  array.finalize();

  Kokkos::RangePolicy<> policy(0, 3);
  KokkosJaggedArrayTestObject1D<3, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int i = 0; i < I + 1; ++i)
      for (unsigned int j = 0; j < I + 1; ++j)
        for (unsigned int k = 0; k < I + 1; ++k)
        {
          unsigned int idx = k + j * (I + 1) + i * (I + 1) * (I + 1);
          EXPECT_EQ(array(I)[idx], i + j + k);
        }
}

TEST_F(KokkosArrayTest, jaggedArray1D2DRightLayout)
{
  Moose::Kokkos::JaggedArray<unsigned int, 1, 2, dof_id_type, Moose::Kokkos::LayoutType::RIGHT>
      array(3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      array.reserve({I, J}, {I + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 3});
  KokkosJaggedArrayTestObject2D<1, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int i = 0; i < I + 1; ++i)
        EXPECT_EQ(array(I, J)[i], i);
}

TEST_F(KokkosArrayTest, jaggedArray2D2DRightLayout)
{
  Moose::Kokkos::JaggedArray<unsigned int, 2, 2, dof_id_type, Moose::Kokkos::LayoutType::RIGHT>
      array(3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      array.reserve({I, J}, {I + 1, J + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 3});
  KokkosJaggedArrayTestObject2D<2, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int i = 0; i < I + 1; ++i)
        for (unsigned int j = 0; j < J + 1; ++j)
        {
          unsigned int idx = j + i * (J + 1);
          EXPECT_EQ(array(I, J)[idx], i + j);
        }
}

TEST_F(KokkosArrayTest, jaggedArray3D2DRightLayout)
{
  Moose::Kokkos::JaggedArray<unsigned int, 3, 2, dof_id_type, Moose::Kokkos::LayoutType::RIGHT>
      array(3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      array.reserve({I, J}, {I + 1, J + 1, J + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<2>> policy({0, 0}, {3, 3});
  KokkosJaggedArrayTestObject2D<3, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int i = 0; i < I + 1; ++i)
        for (unsigned int j = 0; j < J + 1; ++j)
          for (unsigned int k = 0; k < J + 1; ++k)
          {
            unsigned int idx = k + j * (J + 1) + i * (J + 1) * (J + 1);
            EXPECT_EQ(array(I, J)[idx], i + j + k);
          }
}

TEST_F(KokkosArrayTest, jaggedArray1D3DRightLayout)
{
  Moose::Kokkos::JaggedArray<unsigned int, 1, 3, dof_id_type, Moose::Kokkos::LayoutType::RIGHT>
      array(3, 3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        array.reserve({I, J, K}, {I + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 3, 3});
  KokkosJaggedArrayTestObject3D<1, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        for (unsigned int i = 0; i < I + 1; ++i)
          EXPECT_EQ(array(I, J, K)[i], i);
}

TEST_F(KokkosArrayTest, jaggedArray2D3DRightLayout)
{
  Moose::Kokkos::JaggedArray<unsigned int, 2, 3, dof_id_type, Moose::Kokkos::LayoutType::RIGHT>
      array(3, 3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        array.reserve({I, J, K}, {I + 1, J + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 3, 3});
  KokkosJaggedArrayTestObject3D<2, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        for (unsigned int i = 0; i < I + 1; ++i)
          for (unsigned int j = 0; j < J + 1; ++j)
          {
            unsigned int idx = j + i * (J + 1);
            EXPECT_EQ(array(I, J, K)[idx], i + j);
          }
}

TEST_F(KokkosArrayTest, jaggedArray3D3DRightLayout)
{
  Moose::Kokkos::JaggedArray<unsigned int, 3, 3, dof_id_type, Moose::Kokkos::LayoutType::RIGHT>
      array(3, 3, 3);

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        array.reserve({I, J, K}, {I + 1, J + 1, K + 1});

  array.finalize();

  Kokkos::MDRangePolicy<Kokkos::Rank<3>> policy({0, 0, 0}, {3, 3, 3});
  KokkosJaggedArrayTestObject3D<3, dof_id_type, Moose::Kokkos::LayoutType::RIGHT> object(array);
  Kokkos::parallel_for(policy, object);

  array.copyToHost();

  for (unsigned int I = 0; I < 3; ++I)
    for (unsigned int J = 0; J < 3; ++J)
      for (unsigned int K = 0; K < 3; ++K)
        for (unsigned int i = 0; i < I + 1; ++i)
          for (unsigned int j = 0; j < J + 1; ++j)
            for (unsigned int k = 0; k < K + 1; ++k)
            {
              unsigned int idx = k + j * (K + 1) + i * (K + 1) * (J + 1);
              EXPECT_EQ(array(I, J, K)[idx], i + j + k);
            }
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

TEST_F(KokkosArrayTest, blasAxby)
{
  Moose::Kokkos::Array<Real> x(3);
  Moose::Kokkos::Array<Real> y(3);
  Moose::Kokkos::Array<Real> array(3);

  x[0] = 1.0;
  x[1] = 3.0;
  x[2] = 5.0;

  y[0] = 2.0;
  y[1] = 4.0;
  y[2] = 6.0;

  x.copyToDevice();
  y.copyToDevice();

  array.axby(2.0, x, '+', 3.0, y);
  array.copyToHost();

  EXPECT_EQ(array[0], 2.0 * 1.0 + 3.0 * 2.0);
  EXPECT_EQ(array[1], 2.0 * 3.0 + 3.0 * 4.0);
  EXPECT_EQ(array[2], 2.0 * 5.0 + 3.0 * 6.0);

  array.axby(2.0, x, '-', 3.0, y);
  array.copyToHost();

  EXPECT_EQ(array[0], 2.0 * 1.0 - 3.0 * 2.0);
  EXPECT_EQ(array[1], 2.0 * 3.0 - 3.0 * 4.0);
  EXPECT_EQ(array[2], 2.0 * 5.0 - 3.0 * 6.0);

  array.axby(2.0, x, '*', 3.0, y);
  array.copyToHost();

  EXPECT_EQ(array[0], 2.0 * 1.0 * 3.0 * 2.0);
  EXPECT_EQ(array[1], 2.0 * 3.0 * 3.0 * 4.0);
  EXPECT_EQ(array[2], 2.0 * 5.0 * 3.0 * 6.0);

  array.axby(2.0, x, '/', 3.0, y);
  array.copyToHost();

  EXPECT_EQ(array[0], (2.0 * 1.0) / (3.0 * 2.0));
  EXPECT_EQ(array[1], (2.0 * 3.0) / (3.0 * 4.0));
  EXPECT_EQ(array[2], (2.0 * 5.0) / (3.0 * 6.0));
}

TEST_F(KokkosArrayTest, blasAxbyAccumulate)
{
  Moose::Kokkos::Array<Real> x(3);
  Moose::Kokkos::Array<Real> y(3);
  Moose::Kokkos::Array<Real> array(3);

  x[0] = 1.0;
  x[1] = 3.0;
  x[2] = 5.0;

  y[0] = 2.0;
  y[1] = 4.0;
  y[2] = 6.0;

  x.copyToDevice();
  y.copyToDevice();

  array = 1.0;
  array.axby(2.0, x, '+', 3.0, y, true);
  array.copyToHost();

  EXPECT_EQ(array[0], 1.0 + 2.0 * 1.0 + 3.0 * 2.0);
  EXPECT_EQ(array[1], 1.0 + 2.0 * 3.0 + 3.0 * 4.0);
  EXPECT_EQ(array[2], 1.0 + 2.0 * 5.0 + 3.0 * 6.0);

  array = 1.0;
  array.axby(2.0, x, '-', 3.0, y, true);
  array.copyToHost();

  EXPECT_EQ(array[0], 1.0 + 2.0 * 1.0 - 3.0 * 2.0);
  EXPECT_EQ(array[1], 1.0 + 2.0 * 3.0 - 3.0 * 4.0);
  EXPECT_EQ(array[2], 1.0 + 2.0 * 5.0 - 3.0 * 6.0);

  array = 1.0;
  array.axby(2.0, x, '*', 3.0, y, true);
  array.copyToHost();

  EXPECT_EQ(array[0], 1.0 + 2.0 * 1.0 * 3.0 * 2.0);
  EXPECT_EQ(array[1], 1.0 + 2.0 * 3.0 * 3.0 * 4.0);
  EXPECT_EQ(array[2], 1.0 + 2.0 * 5.0 * 3.0 * 6.0);

  array = 1.0;
  array.axby(2.0, x, '/', 3.0, y, true);
  array.copyToHost();

  EXPECT_EQ(array[0], 1.0 + (2.0 * 1.0) / (3.0 * 2.0));
  EXPECT_EQ(array[1], 1.0 + (2.0 * 3.0) / (3.0 * 4.0));
  EXPECT_EQ(array[2], 1.0 + (2.0 * 5.0) / (3.0 * 6.0));
}

TEST_F(KokkosArrayTest, blasScal)
{
  Moose::Kokkos::Array<Real> array(3);

  array[0] = 1.0;
  array[1] = 2.0;
  array[2] = 3.0;

  array.copyToDevice();
  array.scal(2.0);
  array.copyToHost();

  EXPECT_EQ(array[0], 2.0 * 1.0);
  EXPECT_EQ(array[1], 2.0 * 2.0);
  EXPECT_EQ(array[2], 2.0 * 3.0);
}

TEST_F(KokkosArrayTest, blasDot)
{
  Moose::Kokkos::Array<Real> array(3);

  array[0] = 1.0;
  array[1] = 2.0;
  array[2] = 3.0;

  array.copyToDevice();
  Real dot = array.dot(array);

  EXPECT_EQ(dot, 1.0 * 1.0 + 2.0 * 2.0 + 3.0 * 3.0);
}

TEST_F(KokkosArrayTest, blasNrm2)
{
  Moose::Kokkos::Array<Real> array(3);

  array[0] = 1.0;
  array[1] = 2.0;
  array[2] = 3.0;

  array.copyToDevice();
  Real nrm2 = array.nrm2();

  EXPECT_EQ(nrm2, std::sqrt(1.0 * 1.0 + 2.0 * 2.0 + 3.0 * 3.0));
}
