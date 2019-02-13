//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

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
