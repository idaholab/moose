//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseArray.h"

using libMesh::Real;

TEST(MooseArray, defaultConstructor)
{
  MooseArray<int> ma;
  EXPECT_EQ(ma.size(), 0);
}

TEST(MooseArray, sizeConstructor)
{
  MooseArray<int> ma(6);
  EXPECT_EQ(ma.size(), 6);
  ma[5] = 42;
  EXPECT_EQ(ma[5], 42);

  ma.release();
}

TEST(MooseArray, valueConstructor)
{
  int value = 42;
  MooseArray<int> ma(6, value);
  EXPECT_EQ(ma[0], 42);
  EXPECT_EQ(ma[1], 42);
  EXPECT_EQ(ma[2], 42);
  EXPECT_EQ(ma[3], 42);
  EXPECT_EQ(ma[4], 42);
  EXPECT_EQ(ma[5], 42);
  EXPECT_EQ(ma.size(), 6);

  ma[5] = 44;
  EXPECT_EQ(ma[5], 44);

  ma.release();
}

TEST(MooseArray, setAllValues)
{
  MooseArray<int> ma(6);
  ma[5] = 44;
  int value = 42;
  ma.setAllValues(value);

  EXPECT_EQ(ma[0], 42);
  EXPECT_EQ(ma[1], 42);
  EXPECT_EQ(ma[2], 42);
  EXPECT_EQ(ma[3], 42);
  EXPECT_EQ(ma[4], 42);
  EXPECT_EQ(ma[5], 42);
  EXPECT_EQ(ma.size(), 6);

  ma.release();
}

TEST(MooseArray, release)
{
  MooseArray<int> ma(6);
  EXPECT_EQ(ma.size(), 6);
  ma.release();
  EXPECT_EQ(ma.size(), 0);
}

TEST(MooseArray, resize)
{
  MooseArray<int> ma(4);
  ma[0] = 1;
  ma[1] = 2;
  ma[2] = 3;
  ma[3] = 4;

  EXPECT_EQ(ma.size(), 4);
  ma.resize(6, 42);
  EXPECT_EQ(ma.size(), 6);
  EXPECT_EQ(ma[4], 42);
  EXPECT_EQ(ma[5], 42);
  ma[0] = 1;
  ma[5] = 44;
  EXPECT_EQ(ma[0], 1);
  EXPECT_EQ(ma[5], 44);

  ma.resize(2, 44);
  EXPECT_EQ(ma.size(), 2);

  ma.resize(4, 33);
  EXPECT_EQ(ma.size(), 4);

  // These tests only pass if resize() sets default_value works when resizing
  // to a value still less than _allocated_size
  // EXPECT_EQ( ma[2], 33 );
  // EXPECT_EQ( ma[3], 33 );

  ma.release();
}

TEST(MooseArray, resizeDefault)
{
  MooseArray<int> ma(4);
  ma[0] = 1;
  ma[1] = 2;
  ma[2] = 3;
  ma[3] = 4;

  EXPECT_EQ(ma.size(), 4);
  ma.resize(6);
  ma[0] = 1;
  ma[5] = 42;
  EXPECT_EQ(ma.size(), 6);
  EXPECT_EQ(ma[0], 1);
  EXPECT_EQ(ma[5], 42);

  ma.resize(2);
  EXPECT_EQ(ma.size(), 2);
  ma[0] = 1;
  EXPECT_EQ(ma[0], 1);

  ma.release();
}

TEST(MooseArray, size)
{
  // mostly tested in other functions
  MooseArray<int> ma(6);
  EXPECT_EQ(ma.size(), 6);

  ma.release();
}

TEST(MooseArray, access)
{
  MooseArray<int> ma(4);
  ma[0] = 1;
  ma[1] = 2;
  ma[2] = 3;
  ma[3] = 4;

  EXPECT_EQ(ma[0], 1);
  EXPECT_EQ(ma[1], 2);
  EXPECT_EQ(ma[2], 3);
  EXPECT_EQ(ma[3], 4);

  ma.release();
}

TEST(MooseArray, shallowCopy)
{
  // shallow copy a few different sizes of arrays and make sure the sizes and values stay consistent
  MooseArray<Real> ma4(4, 8);
  MooseArray<Real> ma3(3);
  ma3[0] = 1;
  ma3[1] = 2;
  ma3[2] = 3;
  MooseArray<Real> ma2(2, 9);

  // We need a few extra MooseArray's around to keep track of memory
  MooseArray<Real> ma_tmp1, ma_tmp2;

  ma_tmp1.shallowCopy(ma2);
  ma_tmp2.shallowCopy(ma4);

  ma4.shallowCopy(ma3);
  ma2.shallowCopy(ma3);

  EXPECT_EQ(ma4.size(), 3);
  EXPECT_EQ(ma4[0], 1);
  EXPECT_EQ(ma4[1], 2);
  EXPECT_EQ(ma4[2], 3);
  EXPECT_EQ(ma2.size(), 3);
  EXPECT_EQ(ma2[0], 1);
  EXPECT_EQ(ma2[1], 2);
  EXPECT_EQ(ma2[2], 3);

  // Resize which will trigger another allocation, but the first is cleaned up
  ma4.resize(5, 42);

  // More shallow copies
  ma2.shallowCopy(ma4);
  ma3.shallowCopy(ma4);
  ma4[0] = 22;

  EXPECT_EQ(ma4.size(), 5);
  EXPECT_EQ(ma2.size(), 5);
  EXPECT_EQ(ma3.size(), 5);
  EXPECT_EQ(ma2[4], 42);
  EXPECT_EQ(ma3[4], 42);
  EXPECT_EQ(ma4[4], 42);
  EXPECT_EQ(ma2[0], 22);
  EXPECT_EQ(ma3[0], 22);
  EXPECT_EQ(ma4[0], 22);

  // Cleanup
  ma_tmp1.release();
  ma_tmp2.release();
  ma4.release();
}

TEST(MooseArray, shallowCopyStdVector)
{
  std::vector<Real> avec;
  avec.push_back(1.2);
  avec.push_back(3.4);
  avec.push_back(6.7);

  MooseArray<Real> ma;

  ma.shallowCopy(avec);

  EXPECT_EQ(ma[0], 1.2);
  EXPECT_EQ(ma[1], 3.4);
  EXPECT_EQ(ma[2], 6.7);
}

TEST(MooseArray, operatorEqualsStdVector)
{
  std::vector<Real> avec;
  avec.push_back(1.2);
  avec.push_back(3.4);
  avec.push_back(6.7);

  MooseArray<Real> ma;

  ma = avec;

  EXPECT_EQ(ma[0], 1.2);
  EXPECT_EQ(ma[1], 3.4);
  EXPECT_EQ(ma[2], 6.7);

  ma.release();
}

TEST(MooseArray, stdVector)
{
  MooseArray<Real> ma(3);
  ma[0] = 1.2;
  ma[1] = 3.4;
  ma[2] = 6.7;

  std::vector<Real> avec = ma.stdVector();

  EXPECT_EQ(avec[0], 1.2);
  EXPECT_EQ(avec[1], 3.4);
  EXPECT_EQ(avec[2], 6.7);

  ma.release();
}
