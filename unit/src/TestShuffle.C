//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "gtest/gtest.h"
#include "MooseRandom.h"
#include "Shuffle.h"
#include "libmesh/communicator.h"
#include <vector>

using namespace MooseUtils;

TEST(MooseUtils, swap)
{
  std::vector<int> x = {0, 1, 2, 3, 4};
  MooseUtils::swap(x, 2, 4);
  EXPECT_EQ(x, std::vector<int>({0, 1, 4, 3, 2}));

  MooseUtils::swap(x, 0, 3);
  EXPECT_EQ(x, std::vector<int>({3, 1, 4, 0, 2}));
}

TEST(MooseUtils, shuffle)
{
  MooseRandom generator;
  generator.seed(0, 1980);
  generator.seed(1, 1949);
  generator.saveState();
  const std::vector<int> gold_1980 = {6, 3, 8, 7, 2, 4, 1, 5, 9, 0};
  const std::vector<int> gold_1949 = {6, 4, 7, 5, 9, 0, 1, 3, 2, 8};

  {
    std::vector<int> x = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    shuffle<int>(x, generator);
    EXPECT_EQ(x, gold_1980);
  }

  {
    std::vector<int> x = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    shuffle<int>(x, generator, 1);
    EXPECT_EQ(x, gold_1949);
  }
}

TEST(MooseUtils, resample)
{
  MooseRandom generator;
  generator.seed(0, 1980);
  generator.seed(1, 1949);

  const std::vector<int> x = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  const std::vector<int> gold_1980 = {1, 1, 7, 2, 9, 6, 6, 0, 8, 9};
  const std::vector<int> gold_1949 = {9, 2, 5, 2, 0, 7, 1, 2, 1, 1};

  std::vector<int> out;
  out = resample<int>(x, generator);
  EXPECT_EQ(out, gold_1980);

  out = resample<int>(x, generator, 1);
  EXPECT_EQ(out, gold_1949);
}

TEST(MooseUtils, resampleWithFunctor)
{
  MooseRandom generator;
  generator.seed(0, 1980);
  generator.seed(1, 1949);

  const std::vector<int> x = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  const std::vector<int> gold_1980 = {1, 1, 7, 2, 9, 6, 6, 0, 8, 9};
  const std::vector<int> gold_1949 = {9, 2, 5, 2, 0, 7, 1, 2, 1, 1};

  std::vector<int> out;
  auto act = [&out](const int & val) { out.push_back(val); };

  resampleWithFunctor(x, act, generator);
  EXPECT_EQ(out, gold_1980);

  out.clear();
  resampleWithFunctor(x, act, generator, 1);
  EXPECT_EQ(out, gold_1949);
}
