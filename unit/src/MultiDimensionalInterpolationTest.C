//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiIndex.h"
#include "MultiDimensionalInterpolation.h"

#include <gtest/gtest.h>

TEST(MultiDimensionalInterpolationTest, linearSearch)
{
  // Construct a 3 indexed objects of Reals
  MultiIndex<Real>::size_type shape(3);
  shape[0] = 3;
  shape[1] = 5;
  shape[2] = 4;
  MultiIndex<Real> mindex = MultiIndex<Real>(shape);

  // parenthesis operator
  std::vector<Real> xa = {0, 10, 15};
  std::vector<Real> ya = {-10, -1.5, 12, 100, 200};
  std::vector<Real> za = {10, 11, 1500, 2000};
  MultiIndex<Real>::size_type index(3);
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
      {
        index[0] = j0;
        index[1] = j1;
        index[2] = j2;
        Real x = xa[j0];
        Real y = ya[j1];
        Real z = za[j2];
        mindex(index) = x * x + 3 * y - 5 * std::sqrt(z);
      }

  // define base_point array
  std::vector<std::vector<Real>> bp = {xa, ya, za};

  // construct interpolation object using constructor 1 & then set data
  {
    MultiDimensionalInterpolation interp;
    interp.setData(bp, mindex);
    std::vector<Real> values = {12.5, -2.1, 4000};
    MultiIndex<Real>::size_type indices;
    interp.linearSearch(values, indices);
    EXPECT_EQ(indices[0], 1);
    EXPECT_EQ(indices[1], 0);
    EXPECT_EQ(indices[2], 2);
    EXPECT_EQ(values[2], 2000);

    values = {-1000, -2.1, 4000};
    interp.linearSearch(values, indices);
    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 0);
    EXPECT_EQ(indices[2], 2);
    EXPECT_EQ(values[0], 0);

    values = {0, -1.5, 2000};
    interp.linearSearch(values, indices);
    EXPECT_EQ(indices[0], 0);
    EXPECT_EQ(indices[1], 1);
    EXPECT_EQ(indices[2], 2);
  }

  // construct interpolation object using constructor 2
  {
    MultiDimensionalInterpolation interp(bp, mindex);
    std::vector<Real> values = {12.5, -2.1, 4000};
    MultiIndex<Real>::size_type indices;
    interp.linearSearch(values, indices);
    EXPECT_EQ(indices[0], 1);
    EXPECT_EQ(indices[1], 0);
    EXPECT_EQ(indices[2], 2);
    EXPECT_EQ(values[2], 2000);
  }
}

TEST(MultiDimensionalInterpolationTest, interpolate)
{
  // Construct a 3 indexed objects of Reals
  MultiIndex<Real>::size_type shape(3);
  shape[0] = 3;
  shape[1] = 5;
  shape[2] = 4;
  MultiIndex<Real> mindex = MultiIndex<Real>(shape);

  // linear function should be interpolated exactly
  std::vector<Real> xa = {0, 1, 3};
  std::vector<Real> ya = {-2, 5, 6, 7, 12};
  std::vector<Real> za = {10, 11, 12.5, 13};
  MultiIndex<Real>::size_type index(3);
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
      {
        index[0] = j0;
        index[1] = j1;
        index[2] = j2;
        Real x = xa[j0];
        Real y = ya[j1];
        Real z = za[j2];
        mindex(index) = 4 + x + 2 * y - 5 * z + x * y + 2 * y * z + 0.5 * x * y * z;
      }

  std::vector<std::vector<Real>> bp = {xa, ya, za};
  MultiDimensionalInterpolation interp(bp, mindex);
  unsigned int np = 10;
  for (unsigned int jx = 0; jx < np + 1; ++jx)
    for (unsigned int jy = 0; jy < np + 1; ++jy)
      for (unsigned int jz = 0; jz < np + 1; ++jz)
      {
        Real x = 3.0 / np * jx;
        Real y = 14.0 / np * jy - 2;
        Real z = 3.0 / np * jz + 10;
        Real ival = interp.multiLinearInterpolation({x, y, z});
        Real val = 4 + x + 2 * y - 5 * z + x * y + 2 * y * z + 0.5 * x * y * z;
        EXPECT_NEAR(ival, val, 1e-12);
      }

  // nonlinear function is interpolated with small error
  xa = {1, 1.01, 1.03};
  ya = {-0.02, 0, 0.05, 0.1, 0.2};
  za = {10, 10.01, 10.02, 10.05};
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
      {
        index[0] = j0;
        index[1] = j1;
        index[2] = j2;
        Real x = xa[j0];
        Real y = ya[j1];
        Real z = za[j2];
        mindex(index) = std::sqrt(z) * x * x + y * y * y;
      }

  bp = {xa, ya, za};
  interp.setData(bp, mindex);
  np = 10;
  for (unsigned int jx = 0; jx < np + 1; ++jx)
    for (unsigned int jy = 0; jy < np + 1; ++jy)
      for (unsigned int jz = 0; jz < np + 1; ++jz)
      {
        Real x = 0.03 / np * jx + 1;
        Real y = 0.22 / np * jy - 0.02;
        Real z = 0.05 / np * jz + 10;
        Real ival = interp.multiLinearInterpolation({x, y, z});
        Real val = std::sqrt(z) * x * x + y * y * y;
        Real err = std::abs(1 - ival / val) * 100;
        EXPECT_TRUE(err < 10);
      }
}

TEST(MultiDimensionalInterpolationTest, interpolateCornerCases)
{
  // Construct a 3 indexed objects of Reals
  MultiIndex<Real>::size_type shape(5);
  shape[0] = 3;
  shape[1] = 1;
  shape[2] = 5;
  shape[3] = 4;
  shape[4] = 1;
  MultiIndex<Real> mindex = MultiIndex<Real>(shape);

  // linear function should be interpolated exactly
  std::vector<Real> ua = {0, 1, 3};
  std::vector<Real> wa = {12};
  std::vector<Real> xa = {-2, 5, 6, 7, 12};
  std::vector<Real> ya = {10, 11, 12.5, 13};
  std::vector<Real> za = {-50};
  MultiIndex<Real>::size_type index(5);
  for (unsigned int j0 = 0; j0 < shape[0]; ++j0)
    for (unsigned int j1 = 0; j1 < shape[1]; ++j1)
      for (unsigned int j2 = 0; j2 < shape[2]; ++j2)
        for (unsigned int j3 = 0; j3 < shape[3]; ++j3)
          for (unsigned int j4 = 0; j4 < shape[4]; ++j4)
          {
            index = {j0, j1, j2, j3, j4};
            Real u = ua[j0];
            Real w = wa[j1];
            Real x = xa[j2];
            Real y = ya[j3];
            Real z = za[j4];
            mindex(index) = 4 + 10 * u - 12 * w + x + 2 * y - 5 * z + x * y + 2 * y * z +
                            0.5 * x * y * z * u * w;
          }

  std::vector<std::vector<Real>> bp = {ua, wa, xa, ya, za};
  MultiDimensionalInterpolation interp(bp, mindex);

  // first point
  std::vector<Real> pt = {3.5, 100, -4, 10.5, 1e12};
  Real u = 3;
  Real w = 12;
  Real x = -2;
  Real y = 10.5;
  Real z = -50;
  Real val = 4 + 10 * u - 12 * w + x + 2 * y - 5 * z + x * y + 2 * y * z + 0.5 * x * y * z * u * w;
  Real ival = interp.multiLinearInterpolation(pt);
  EXPECT_EQ(ival, val);
}

TEST(MultiDimensionalInterpolationTest, interpolateDegenerateCase)
{
  // Construct a 3 indexed objects of Reals
  MultiIndex<Real>::size_type shape = {1, 1, 1, 1, 1};
  MultiIndex<Real> mindex = MultiIndex<Real>(shape, {15});
  std::vector<std::vector<Real>> bp = {{10}, {12}, {-140}, {5e6}, {-1e-11}};
  MultiDimensionalInterpolation interp(bp, mindex);
  std::vector<Real> pt = {20, 15, 36, 12, -1};
  Real ival = interp.multiLinearInterpolation(pt);
  EXPECT_EQ(ival, 15);
}
