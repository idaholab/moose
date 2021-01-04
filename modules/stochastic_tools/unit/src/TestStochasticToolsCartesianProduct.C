//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "CartesianProduct.h"

using namespace StochasticTools;

TEST(StochasticTools, CartesianProduct)
{
  const std::vector<std::vector<Real>> gold = {
      {2, 3, 4},  {2, 3, 16},  {2, 9, 4},  {2, 9, 16},  {2, 27, 4},  {2, 27, 16},
      {4, 3, 4},  {4, 3, 16},  {4, 9, 4},  {4, 9, 16},  {4, 27, 4},  {4, 27, 16},
      {8, 3, 4},  {8, 3, 16},  {8, 9, 4},  {8, 9, 16},  {8, 27, 4},  {8, 27, 16},
      {16, 3, 4}, {16, 3, 16}, {16, 9, 4}, {16, 9, 16}, {16, 27, 4}, {16, 27, 16}};

  const std::vector<std::vector<Real>> x = {{2, 4, 8, 16}, {3, 9, 27}, {4, 16}};

  const CartesianProduct<Real> cp(x);

  EXPECT_EQ(cp.numRows(), (std::size_t)24);
  EXPECT_EQ(cp.numCols(), (std::size_t)3);

  {
    auto out = cp.computeMatrix();
    EXPECT_EQ(out, gold);
  }

  {
    EXPECT_EQ(cp.computeRow(0), gold[0]);
    EXPECT_EQ(cp.computeRow(1), gold[1]);
    EXPECT_EQ(cp.computeRow(2), gold[2]);
    EXPECT_EQ(cp.computeRow(3), gold[3]);
    EXPECT_EQ(cp.computeRow(4), gold[4]);
    EXPECT_EQ(cp.computeRow(5), gold[5]);
    EXPECT_EQ(cp.computeRow(6), gold[6]);
    EXPECT_EQ(cp.computeRow(7), gold[7]);
    EXPECT_EQ(cp.computeRow(8), gold[8]);
    EXPECT_EQ(cp.computeRow(9), gold[9]);
    EXPECT_EQ(cp.computeRow(10), gold[10]);
    EXPECT_EQ(cp.computeRow(11), gold[11]);
    EXPECT_EQ(cp.computeRow(12), gold[12]);
    EXPECT_EQ(cp.computeRow(13), gold[13]);
    EXPECT_EQ(cp.computeRow(14), gold[14]);
    EXPECT_EQ(cp.computeRow(15), gold[15]);
    EXPECT_EQ(cp.computeRow(16), gold[16]);
    EXPECT_EQ(cp.computeRow(17), gold[17]);
    EXPECT_EQ(cp.computeRow(18), gold[18]);
    EXPECT_EQ(cp.computeRow(19), gold[19]);
    EXPECT_EQ(cp.computeRow(20), gold[20]);
    EXPECT_EQ(cp.computeRow(21), gold[21]);
    EXPECT_EQ(cp.computeRow(22), gold[22]);
    EXPECT_EQ(cp.computeRow(23), gold[23]);
  }

  {
    for (std::size_t row = 0; row < cp.numRows(); ++row)
      for (std::size_t col = 0; col < cp.numCols(); ++col)
        EXPECT_EQ(cp.computeValue(row, col), gold[row][col]);
  }
}

TEST(StochasticTools, WeightedCartesianProduct)
{
  const std::vector<Real> gold = {
      1 * 10 * 100, 1 * 10 * 110, 1 * 11 * 100, 1 * 11 * 110, 1 * 12 * 100, 1 * 12 * 110,
      2 * 10 * 100, 2 * 10 * 110, 2 * 11 * 100, 2 * 11 * 110, 2 * 12 * 100, 2 * 12 * 110,
      3 * 10 * 100, 3 * 10 * 110, 3 * 11 * 100, 3 * 11 * 110, 3 * 12 * 100, 3 * 12 * 110,
      4 * 10 * 100, 4 * 10 * 110, 4 * 11 * 100, 4 * 11 * 110, 4 * 12 * 100, 4 * 12 * 110};

  const std::vector<std::vector<Real>> x = {{2, 4, 8, 16}, {3, 9, 27}, {4, 16}};
  const std::vector<std::vector<Real>> w = {{1, 2, 3, 4}, {10, 11, 12}, {100, 110}};

  const WeightedCartesianProduct<Real, Real> wcp(x, w);

  EXPECT_EQ(wcp.numRows(), (std::size_t)24);
  EXPECT_EQ(wcp.numCols(), (std::size_t)3);

  {
    auto out = wcp.computeWeightVector();
    EXPECT_EQ(out, gold);
  }

  {
    for (std::size_t row = 0; row < wcp.numRows(); ++row)
      EXPECT_EQ(wcp.computeWeight(row), gold[row]);
  }
}
