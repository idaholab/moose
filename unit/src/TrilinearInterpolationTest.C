//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "TrilinearInterpolation.h"

#include <cmath>

TEST(TrilinearInterpolationTest, sample1)
{
  std::vector<double> x = {1, 6};
  std::vector<double> y = {2, 7};
  std::vector<double> z = {3, 8};
  // clang-format off
  std::vector<double> data = {// x = 1
                              3, 15,
                              8, 17,
                              // x = 6
                              5, 16,
                              13, 18};
  // clang-format on
  TrilinearInterpolation tli(x, y, z, data);

  // testing corners (8)
  EXPECT_NEAR(tli.sample(1, 2, 3), 3, 1e-15);
  EXPECT_NEAR(tli.sample(1, 2, 8), 15, 1e-15);
  EXPECT_NEAR(tli.sample(6, 2, 8), 16, 1e-15);
  EXPECT_NEAR(tli.sample(6, 2, 3), 5, 1e-15);
  EXPECT_NEAR(tli.sample(1, 7, 3), 8, 1e-15);
  EXPECT_NEAR(tli.sample(1, 7, 8), 17, 1e-15);
  EXPECT_NEAR(tli.sample(6, 7, 8), 18, 1e-15);
  EXPECT_NEAR(tli.sample(6, 7, 3), 13, 1e-15);

  // testing points on edges (12)
  EXPECT_NEAR(tli.sample(1, 2, 4), 5.4, 1e-15);
  EXPECT_NEAR(tli.sample(2, 2, 8), 15.2, 1e-15);
  EXPECT_NEAR(tli.sample(6, 2, 7), 13.8, 1e-15);
  EXPECT_NEAR(tli.sample(3, 2, 3), 3.8, 1e-15);
  EXPECT_NEAR(tli.sample(1, 3, 8), 15.4, 1e-15);
  EXPECT_NEAR(tli.sample(6, 4, 8), 16.8, 1e-15);
  EXPECT_NEAR(tli.sample(6, 5, 3), 9.8, 1e-15);
  EXPECT_NEAR(tli.sample(1, 6, 3), 7, 1e-15);
  EXPECT_NEAR(tli.sample(4, 7, 8), 17.6, 1e-15);
  EXPECT_NEAR(tli.sample(6, 7, 6), 16, 1e-15);
  EXPECT_NEAR(tli.sample(5, 7, 3), 12, 1e-15);
  EXPECT_NEAR(tli.sample(1, 7, 5), 11.6, 1e-15);

  // testing faces (6)
  EXPECT_NEAR(tli.sample(1, 3, 4), 6.28, 1e-15);
  EXPECT_NEAR(tli.sample(2, 2, 5), 8.12, 1e-15);
  EXPECT_NEAR(tli.sample(6, 4, 6), 13.36, 1e-15);
  EXPECT_NEAR(tli.sample(3, 7, 7), 15.92, 2e-15);
  EXPECT_NEAR(tli.sample(4, 5, 3), 8.28, 2e-15);
  EXPECT_NEAR(tli.sample(5, 6, 8), 17.40, 5e-15);

  // testing point inside cube
  EXPECT_NEAR(tli.sample(3, 4, 5), 10.248, 1e-15);
}

TEST(TrilinearInterpolationTest, sample2)
{
  std::vector<double> x = {1};
  std::vector<double> y = {2};
  std::vector<double> z = {3};
  std::vector<double> data = {7};
  TrilinearInterpolation tli(x, y, z, data);

  // testing for values around and at a single point
  EXPECT_NEAR(tli.sample(1, 2, 1), 7, 1e-15);
  EXPECT_NEAR(tli.sample(1, 2, 5), 7, 1e-15);
  EXPECT_NEAR(tli.sample(0, 2, 3), 7, 1e-15);
  EXPECT_NEAR(tli.sample(4, 2, 3), 7, 1e-15);
  EXPECT_NEAR(tli.sample(1, 0, 3), 7, 1e-15);
  EXPECT_NEAR(tli.sample(1, 6, 3), 7, 1e-15);

  EXPECT_NEAR(tli.sample(2, 1, 3), 7, 1e-15);

  // actual point
  EXPECT_NEAR(tli.sample(1, 2, 3), 7, 1e-15);
}

TEST(TrilinearInterpolationTest, errorCatch)
{
  std::vector<Real> x = {1, 2, 3};
  std::vector<Real> y = {4, 5, 6};
  std::vector<Real> z = {7, 8, 9};

  try
  {
    std::vector<Real> x1 = {1, 2};
    // clang-format off
    std::vector<Real> data1 = {
                              // fxyz for x = 1
                              10, 11, 12,
                              13, 14, 15,
                              16, 17, 18,
                              // fxyz for x = 2
                              20, 21, 22,
                              23, 24, 25,
                              26, 27, 28
                              };
    // clang-format on
    TrilinearInterpolation tli1(x1, y, z, data1);
  }
  catch (const std::exception & E)
  {
    std::string msg(E.what());
    EXPECT_TRUE(msg.find("x vector has zero elements. At least one element is required.") !=
                std::string::npos);
  }

  try
  {
    std::vector<Real> y1 = {4, 5};
    // clang-format off
    std::vector<Real> data2 = {
                              // fxyz for x = 1
                              10, 11,
                              13, 14,
                              16, 17,
                              // fxyz for x = 2
                              20, 21,
                              23, 24,
                              26, 27,
                              // fxyz for x = 3
                              30, 31,
                              33, 34,
                              36, 37
                              };
    // clang-format on
    TrilinearInterpolation tli2(x, y1, z, data2);
  }
  catch (const std::exception & E)
  {
    std::string msg(E.what());
    EXPECT_TRUE(msg.find("y vector has zero elements. At least one element is required.") !=
                std::string::npos);
  }

  try
  {
    std::vector<Real> z1 = {7, 8};
    // clang-format off
    std::vector<Real> data3 = {
                              // fxyz for x = 1
                              10, 11, 12,
                              13, 14, 15,
                              // fxyz for x = 2
                              20, 21, 22,
                              23, 24, 25,
                              // fxyz for x = 3
                              30, 31, 32,
                              33, 34, 35
                              };
    // clang-format on
    TrilinearInterpolation tli2(x, y, z1, data3);
  }
  catch (const std::exception & E)
  {
    std::string msg(E.what());
    EXPECT_TRUE(msg.find("z vector has zero elements. At least one element is required.") !=
                std::string::npos);
  }

  try
  {
    // clang-format off
    std::vector<Real> data4 = {
                              // fxyz for x = 1
                              10, 11, 12,
                              13, 14, 15,
                              16, 17, 18,
                              // fxyz for x = 2
                              20, 21, 22,
                              23, 24, 25,
                              26, 27, 28,
                              // fxyz for x = 3
                              30, 31, 32,
                              33, 34, 35,
                              36, 37
                              };
    // clang-format on
    TrilinearInterpolation tli2(x, y, z, data4);
  }
  catch (const std::exception & E)
  {
    std::string msg(E.what());
    EXPECT_TRUE(
        msg.find("The size of data (26) does not match the supplied dimensions (3, 3, 3)") !=
        std::string::npos);
  }
}
