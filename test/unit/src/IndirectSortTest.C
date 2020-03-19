//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

// Moose includes
#include "IndirectSort.h"

TEST(IndirectSort, realSort)
{
  double a1[] = {1.2, 2.3, 1.4, 5.2, 3.4, 7.5, 1.5, 3.14};
  std::vector<double> a(a1, a1 + 8);

  std::vector<size_t> b;
  Moose::indirectSort(a.begin(), a.end(), b);

  EXPECT_EQ(b[0], 0);
  EXPECT_EQ(b[1], 2);
  EXPECT_EQ(b[2], 6);
  EXPECT_EQ(b[3], 1);
  EXPECT_EQ(b[4], 7);
  EXPECT_EQ(b[5], 4);
  EXPECT_EQ(b[6], 3);
  EXPECT_EQ(b[7], 5);
}

TEST(IndirectSort, intSort)
{
  const unsigned length = 8;

  std::vector<int> a(length);
  for (unsigned int i = 0; i < length; ++i)
    a[i] = length - i - 1;

  std::vector<size_t> b;
  Moose::indirectSort(a.begin(), a.end(), b);

  EXPECT_EQ(b[0], 7);
  EXPECT_EQ(b[1], 6);
  EXPECT_EQ(b[2], 5);
  EXPECT_EQ(b[3], 4);
  EXPECT_EQ(b[4], 3);
  EXPECT_EQ(b[5], 2);
  EXPECT_EQ(b[6], 1);
  EXPECT_EQ(b[7], 0);
}

TEST(IndirectSort, testStableSort)
{
  const int length = 4;

  std::vector<std::pair<int, int>> a;
  std::vector<size_t> b;
  a.reserve(length);

  a.push_back(std::make_pair(3, 1));
  a.push_back(std::make_pair(2, 2));
  a.push_back(std::make_pair(2, 3));
  a.push_back(std::make_pair(1, 4));

  Moose::indirectSort(a.begin(), a.end(), b);

  EXPECT_EQ(b[0], 3);
  EXPECT_EQ(b[1], 1);
  EXPECT_EQ(b[2], 2);
  EXPECT_EQ(b[3], 0);
}

TEST(IndirectSort, testDoubleSort)
{
  const int length = 5;

  std::vector<int> a(length);

  a[0] = 154;
  a[1] = 20;
  a[2] = 36;
  a[3] = 4;
  a[4] = 81;

  std::vector<size_t> b;
  Moose::indirectSort(a.begin(), a.end(), b, std::greater<int>());

  std::vector<size_t> c(b.size());

  // This operation is equivalent to doing another sort of the indicies
  for (unsigned int i = 0; i < b.size(); ++i)
    c[b[i]] = i;
  // Moose::indirectSort(b.begin(), b.end(), c);

  EXPECT_EQ(c[0], 0);
  EXPECT_EQ(c[1], 3);
  EXPECT_EQ(c[2], 2);
  EXPECT_EQ(c[3], 4);
  EXPECT_EQ(c[4], 1);
}
