//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <vector>
#include <numeric>

#include "gtest/gtest.h"

#include "Enumerate.h"

void
const_container(const std::vector<int> & v, int value, int counter)
{
  for (auto it : Moose::enumerate(v))
  {
    EXPECT_EQ(it.value(), value++);
    EXPECT_EQ(it.index(), counter++);
  }
}

TEST(Enumerate, container)
{
  std::vector<int> v(10);

  int value = -4;
  std::iota(v.begin(), v.end(), value);

  int counter = 0;
  for (auto it : Moose::enumerate(v))
  {
    EXPECT_EQ(it.value(), value++);
    EXPECT_EQ(it.index(), counter++);
  }

  // Test in const context
  const_container(v, -4, 0);

  // Test modification of values
  value = -3;
  for (auto it : Moose::enumerate(v))
  {
    it.value()++;
    EXPECT_EQ(it.value(), value++);
  }
}

TEST(Enumerate, range)
{
  std::vector<unsigned int> v(10);

  int value = 1;
  std::iota(v.begin(), v.end(), value);

  for (auto it : Moose::enumerate(v.begin(), v.end(), 1))
    EXPECT_EQ(it.value(), it.index());
}
