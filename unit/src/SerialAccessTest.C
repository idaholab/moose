//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "SerialAccess.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

#include <libmesh/int_range.h>

TEST(SerialAccess, Real)
{
  Real r = 0;
  std::size_t count = 0;
  for (auto & a : Moose::serialAccess(r))
  {
    a = 9.0;
    count++;
  }

  EXPECT_EQ(count, 1);
  EXPECT_EQ(r, 9.0);

  // test constant type
  const auto cr = r;
  for (auto & a : Moose::serialAccess(cr))
    r = a + 11;

  EXPECT_EQ(r, 20.0);
}

TEST(SerialAccess, ADReal)
{
  ADReal r = 0;
  std::size_t count = 0;
  for (auto & a : Moose::serialAccess(r))
  {
    a = 7.0;
    count++;
  }

  EXPECT_EQ(count, 1);
  EXPECT_EQ(r, 7.0);
}

TEST(SerialAccess, RealVectorValue)
{
  RealVectorValue r;
  std::size_t count = 0;
  for (auto & a : Moose::serialAccess(r))
    a = ++count;

  EXPECT_EQ(count, Moose::dim);
  EXPECT_EQ(r, RealVectorValue(1, 2, 3));

  // test constant type
  const auto & cr = r;
  Real sum = 0.0;
  for (auto & a : Moose::serialAccess(cr))
    sum += a;

  EXPECT_EQ(sum, 6);
}

TEST(SerialAccess, ADRealVectorValue)
{
  ADRealVectorValue r;
  std::size_t count = 0;
  for (auto & a : Moose::serialAccess(r))
    a = ++count;

  EXPECT_EQ(count, Moose::dim);
  EXPECT_EQ(r, RealVectorValue(1, 2, 3));
}

TEST(SerialAccess, RankTwoTensor)
{
  RankTwoTensor r;
  std::size_t count = 0;
  for (auto & a : Moose::serialAccess(r))
    a = ++count;

  EXPECT_EQ(count, RankTwoTensor::N2);
  EXPECT_EQ(r, RankTwoTensor(1, 4, 7, 2, 5, 8, 3, 6, 9));
}

TEST(SerialAccess, RankFourTensor)
{
  RankFourTensor r;
  std::size_t count = 0;
  for (auto & a : Moose::serialAccess(r))
    a = ++count;

  EXPECT_EQ(count, RankFourTensor::N4);

  count = 0;
  RankFourTensor g;
  for (const auto i : make_range(Moose::dim))
    for (const auto j : make_range(Moose::dim))
      for (const auto k : make_range(Moose::dim))
        for (const auto l : make_range(Moose::dim))
          g(i, j, k, l) = ++count;

  EXPECT_EQ((r - g).L2norm(), 0.0);

  // test constant AD type
  const ADRankFourTensor cr = r;
  ADReal sum = 0.0;
  for (auto & a : Moose::serialAccess(cr))
    sum += a;

  EXPECT_EQ(sum, 3321);
}
