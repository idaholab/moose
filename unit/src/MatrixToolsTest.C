//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MatrixTools.h"
#include "MooseException.h"

TEST(MatrixToolsTest, matrixInversionTest1)
{
  // The matrix
  // ( 2  2)
  // (0.5 1)
  // has inverse
  // ( 1  -2)
  // (-0.5 2)
  std::vector<PetscScalar> mat2(2 * 2);
  std::vector<std::vector<Real>> m(2);
  for (auto & row : m)
    row.resize(2);

  mat2[0] = m[0][0] = 2.0;
  mat2[1] = m[0][1] = 2.0;
  mat2[2] = m[1][0] = 0.5;
  mat2[3] = m[1][1] = 1.0;

  MatrixTools::inverse(mat2, 2);
  EXPECT_NEAR(1, mat2[0], 1E-5);
  EXPECT_NEAR(-2, mat2[1], 1E-5);
  EXPECT_NEAR(-0.5, mat2[2], 1E-5);
  EXPECT_NEAR(2, mat2[3], 1E-5);

  MatrixTools::inverse(m, m);
  EXPECT_NEAR(1, m[0][0], 1E-5);
  EXPECT_NEAR(-2, m[0][1], 1E-5);
  EXPECT_NEAR(-0.5, m[1][0], 1E-5);
  EXPECT_NEAR(2, m[1][1], 1E-5);
}

TEST(MatrixToolsTest, matrixInversionTest2)
{
  // The matrix
  // (1 2 3)
  // (4 5 6)
  // (7 8 1)
  // is singular
  std::vector<PetscScalar> mat3(3 * 3);
  std::vector<std::vector<Real>> m(3);
  for (auto & row : m)
    row.resize(3);

  mat3[0] = m[0][0] = 1.0;
  mat3[1] = m[0][1] = 2.0;
  mat3[2] = m[0][2] = 3.0;
  mat3[3] = m[1][0] = 4.0;
  mat3[4] = m[1][1] = 5.0;
  mat3[5] = m[1][2] = 6.0;
  mat3[6] = m[2][0] = 7.0;
  mat3[7] = m[2][1] = 8.0;
  mat3[8] = m[2][2] = 9.0;

  EXPECT_THROW(MatrixTools::inverse(mat3, 3), MooseException);
  EXPECT_THROW(MatrixTools::inverse(m, m), MooseException);

  std::vector<std::vector<Real>> m2(2);
  for (auto & row : m)
    row.resize(3);

  EXPECT_THROW(MatrixTools::inverse(m, m2), MooseException);

  std::vector<std::vector<Real>> m3(0);
  EXPECT_THROW(MatrixTools::inverse(m3, m3), MooseException);
}

TEST(MatrixToolsTest, matrixInversionTest3)
{
  // The matrix
  // (1 2 3)
  // (0 1 4)
  // (5 6 0)
  // has inverse
  // (-24 18  5)
  // (20 -15 -4)
  // (-5  4   1)
  std::vector<PetscScalar> mat3(3 * 3);
  std::vector<std::vector<Real>> m(3);
  for (auto & row : m)
    row.resize(3);

  mat3[0] = m[0][0] = 1.0;
  mat3[1] = m[0][1] = 2.0;
  mat3[2] = m[0][2] = 3.0;
  mat3[3] = m[1][0] = 0.0;
  mat3[4] = m[1][1] = 1.0;
  mat3[5] = m[1][2] = 4.0;
  mat3[6] = m[2][0] = 5.0;
  mat3[7] = m[2][1] = 6.0;
  mat3[8] = m[2][2] = 0.0;

  MatrixTools::inverse(mat3, 3);
  EXPECT_NEAR(-24, mat3[0], 1E-5);
  EXPECT_NEAR(18, mat3[1], 1E-5);
  EXPECT_NEAR(5, mat3[2], 1E-5);
  EXPECT_NEAR(20, mat3[3], 1E-5);
  EXPECT_NEAR(-15, mat3[4], 1E-5);
  EXPECT_NEAR(-4, mat3[5], 1E-5);
  EXPECT_NEAR(-5, mat3[6], 1E-5);
  EXPECT_NEAR(4, mat3[7], 1E-5);
  EXPECT_NEAR(1, mat3[8], 1E-5);

  MatrixTools::inverse(m, m);
  EXPECT_NEAR(-24, m[0][0], 1E-5);
  EXPECT_NEAR(18, m[0][1], 1E-5);
  EXPECT_NEAR(5, m[0][2], 1E-5);
  EXPECT_NEAR(20, m[1][0], 1E-5);
  EXPECT_NEAR(-15, m[1][1], 1E-5);
  EXPECT_NEAR(-4, m[1][2], 1E-5);
  EXPECT_NEAR(-5, m[2][0], 1E-5);
  EXPECT_NEAR(4, m[2][1], 1E-5);
  EXPECT_NEAR(1, m[2][2], 1E-5);
}
