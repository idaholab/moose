//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include <vector>
#include <array>

#include "MathUtils.h"

TEST(MathUtilsTest, pow)
{
  ASSERT_DOUBLE_EQ(MathUtils::pow(1.2345, 73), std::pow(1.2345, 73));
  ASSERT_DOUBLE_EQ(MathUtils::pow(-0.99542, 58), std::pow(-0.99542, 58));
  ASSERT_DOUBLE_EQ(MathUtils::pow(1.2345, -13), std::pow(1.2345, -13));
  ASSERT_DOUBLE_EQ(MathUtils::pow(-0.99542, -8), std::pow(-0.99542, -8));
}

TEST(MathUtilsTest, poly1Log)
{
  std::vector<std::vector<Real>> table = {{0.2, 0.001, 0, -1.60944},
                                          {0.2, 0.001, 1, 5.0},
                                          {0.2, 0.001, 2, -25.0},
                                          {0.2, 0.001, 3, 250.0},
                                          {0.001, 0.002, 0, -6.71461},
                                          {0.001, 0.002, 1, 500.0},
                                          {0.001, 0.002, 2, 0.0},
                                          {0.001, 0.002, 3, 0.0}};

  for (unsigned int i = 0; i < table.size(); ++i)
    EXPECT_NEAR(MathUtils::poly1Log(table[i][0], table[i][1], static_cast<int>(table[i][2])),
                table[i][3],
                1.e-5);
}

TEST(MathUtilsTest, poly2Log)
{
  std::vector<std::vector<Real>> table = {{0.2, 0.001, 0, -1.60944},
                                          {0.2, 0.001, 1, 5.0},
                                          {0.2, 0.001, 2, -25.0},
                                          {0.2, 0.001, 3, 250.0},
                                          {0.001, 0.002, 0, -6.83961},
                                          {0.001, 0.002, 1, 750.0},
                                          {0.001, 0.002, 2, -250000},
                                          {0.001, 0.002, 3, 0.0}};

  for (unsigned int i = 0; i < table.size(); ++i)
    EXPECT_NEAR(MathUtils::poly2Log(table[i][0], table[i][1], static_cast<int>(table[i][2])),
                table[i][3],
                1.e-5);
}

TEST(MathUtilsTest, poly3Log)
{
  std::vector<std::vector<Real>> table = {{0.2, 0.001, 0, -1.60944},
                                          {0.2, 0.001, 1, 5.0},
                                          {0.2, 0.001, 2, -25.0},
                                          {0.2, 0.001, 3, 250.0},
                                          {0.001, 0.002, 0, -6.88127},
                                          {0.001, 0.002, 1, 875.0},
                                          {0.001, 0.002, 2, -500000},
                                          {0.001, 0.002, 3, 2.5e+08}};

  for (unsigned int i = 0; i < table.size(); ++i)
    EXPECT_NEAR(MathUtils::poly3Log(table[i][0], table[i][1], static_cast<int>(table[i][2])),
                table[i][3],
                1.e-5);
}

TEST(MathUtilsTest, poly4Log)
{
  std::vector<std::vector<Real>> table = {{0.2, 0.001, 0, -1.60944},
                                          {0.2, 0.001, 1, 5.0},
                                          {0.2, 0.001, 2, -25.0},
                                          {0.2, 0.001, 3, 250.0},
                                          {0.001, 0.002, 0, -6.90575},
                                          {0.001, 0.002, 1, 984.375},
                                          {0.001, 0.002, 2, -890625},
                                          {0.001, 0.002, 3, 1.3125e+09}};

  for (unsigned int i = 0; i < table.size(); ++i)
    EXPECT_NEAR(MathUtils::poly4Log(table[i][0], table[i][1], static_cast<int>(table[i][2])),
                table[i][3],
                1.e-5);
}

TEST(MathUtilsTest, TaylorLog)
{
  std::vector<std::vector<Real>> table = {{0.0, -2.5119769},
                                          {0.01, -2.4384881},
                                          {0.1, -1.9049717},
                                          {1.0, 0.0},
                                          {2.5, 0.92910611},
                                          {17.9, 2.8807267},
                                          {123.1, 3.6048257},
                                          {32846.7, 3.7558348}};

  for (unsigned int i = 0; i < table.size(); ++i)
    EXPECT_NEAR(MathUtils::taylorLog(table[i][0]), table[i][1], 1.e-5);
}

TEST(MathUtilsTest, round)
{
  std::vector<std::vector<Real>> table = {
      {0.2, 0}, {0.5, 1}, {0.7, 1}, {1.2, 1}, {10.7, 11}, {-0.1, -0}, {-0.7, -1}, {-10.5, -11}};

  for (unsigned int i = 0; i < table.size(); ++i)
    EXPECT_NEAR(MathUtils::round(table[i][0]), table[i][1], 1.e-5);
}

TEST(MathUtilsTest, poly)
{
  std::vector<Real> table1 = {1, 2, 3, 4, 5};
  std::array<Real, 5> table2 = {1, 2, 3, 4, 5};

  EXPECT_NEAR(MathUtils::poly(table1, 1.5, false), 29.5625, 1e-5);
  EXPECT_NEAR(MathUtils::poly(table1, 1.5, true), 40.0, 1e-5);
  EXPECT_NEAR(MathUtils::poly(table2, 1.5, false), 29.5625, 1e-5);
  EXPECT_NEAR(MathUtils::poly(table2, 1.5, true), 40.0, 1e-5);
}

TEST(MathUtilsTest, polynomial)
{
  std::vector<Real> table1a = {5, 4, 3, 2, 1};
  std::array<Real, 5> table2a = {5, 4, 3, 2, 1};
  std::vector<Real> table1b = {5, 4, 3, 2, 1, 0};
  std::array<Real, 6> table2b = {5, 4, 3, 2, 1, 0};

  // appending a zero coefficient to the list should not change the result
  EXPECT_NEAR(MathUtils::polynomial(table1a, 1.5), MathUtils::polynomial(table1b, 1.5), 1e-5);
  EXPECT_NEAR(MathUtils::polynomial(table2a, 1.5), MathUtils::polynomial(table2b, 1.5), 1e-5);
  EXPECT_NEAR(MathUtils::polynomialDerivative(table1a, 1.5),
              MathUtils::polynomialDerivative(table1b, 1.5),
              1e-5);
  EXPECT_NEAR(MathUtils::polynomialDerivative(table2a, 1.5),
              MathUtils::polynomialDerivative(table2b, 1.5),
              1e-5);

  EXPECT_NEAR(MathUtils::polynomial(table1a, 1.5), 29.5625, 1e-5);
  EXPECT_NEAR(MathUtils::polynomialDerivative(table1a, 1.5), 40.0, 1e-5);
}

TEST(MathUtilsTest, multiIndex)
{
  // Order = 1
  std::vector<std::vector<unsigned int>> mi_13_answer = {{0}, {1}, {2}, {3}};
  std::vector<std::vector<unsigned int>> mi_13 = MathUtils::multiIndex(1, 3);
  for (unsigned int r = 0; r < mi_13.size(); r++)
    for (unsigned int c = 0; c < mi_13[0].size(); c++)
      EXPECT_EQ(mi_13[r][c], mi_13_answer[r][c]);

  // Order = 2
  std::vector<std::vector<unsigned int>> mi_23_answer = {
      {0, 0}, {0, 1}, {1, 0}, {0, 2}, {1, 1}, {2, 0}, {0, 3}, {1, 2}, {2, 1}, {3, 0}};
  std::vector<std::vector<unsigned int>> mi_23 = MathUtils::multiIndex(2, 3);
  for (unsigned int r = 0; r < mi_23.size(); r++)
    for (unsigned int c = 0; c < mi_23[0].size(); c++)
      EXPECT_EQ(mi_23[r][c], mi_23_answer[r][c]);

  // Order = 3
  std::vector<std::vector<unsigned int>> mi_32_answer = {{0, 0, 0},
                                                         {0, 0, 1},
                                                         {0, 1, 0},
                                                         {1, 0, 0},
                                                         {0, 0, 2},
                                                         {0, 1, 1},
                                                         {0, 2, 0},
                                                         {1, 0, 1},
                                                         {1, 1, 0},
                                                         {2, 0, 0}};
  std::vector<std::vector<unsigned int>> mi_32 = MathUtils::multiIndex(3, 2);
  for (unsigned int r = 0; r < mi_32.size(); r++)
    for (unsigned int c = 0; c < mi_32[0].size(); c++)
      EXPECT_EQ(mi_32[r][c], mi_32_answer[r][c]);
}

TEST(MathUtilsTest, linearInterpolation)
{
  using namespace MathUtils;
  EXPECT_NEAR(linearInterpolation<ComputeType::value>(0.15, 0.1, 0.2, 0.4, 1.2), 0.8, 1e-5);
  EXPECT_NEAR(linearInterpolation<ComputeType::derivative>(0.15, 0.1, 0.2, 0.4, 1.2), 8.0, 1e-5);
}

TEST(MathUtilsTest, smootherStep)
{
  using namespace MathUtils;
  const auto start = 1.0;
  const auto end = 1.5;
  const auto x = 1.2;
  const auto smoothed = (x - start) / (end - start);
  const auto val = Utility::pow<3>(smoothed) * (smoothed * (smoothed * 6.0 - 15.0) + 10.0);
  const auto deriv =
      30.0 * Utility::pow<2>(smoothed) * (smoothed * (smoothed - 2.0) + 1.0) / (end - start);
  EXPECT_NEAR(smootherStep<ComputeType::value>(0.0, start, end), 0, 1e-5);
  EXPECT_NEAR(smootherStep<ComputeType::value>(2.0, start, end), 1, 1e-5);
  EXPECT_NEAR(smootherStep<ComputeType::derivative>(0.0, start, end), 0.0, 1e-5);
  EXPECT_NEAR(smootherStep<ComputeType::derivative>(2.0, start, end), 0.0, 1e-5);
  EXPECT_NEAR(smootherStep<ComputeType::value>(x, start, end), val, 1e-5);
  EXPECT_NEAR(smootherStep<ComputeType::derivative>(x, start, end), deriv, 1e-5);
}
