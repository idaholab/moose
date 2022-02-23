//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "libmesh/vector_value.h"

#include "FactorizedRankTwoTensor.h"

#define EXPECT_VEC_NEAR(v1, v2, tol)                                                               \
  {                                                                                                \
    ASSERT_EQ(v1.size(), v2.size());                                                               \
    for (unsigned int i = 0; i < v1.size(); i++)                                                   \
      EXPECT_NEAR(v1[i], v2[i], tol);                                                              \
  }

#define EXPECT_R2T_NEAR(A1, A2, tol)                                                               \
  {                                                                                                \
    for (unsigned int i = 0; i < 3; i++)                                                           \
      for (unsigned int j = 0; j < 3; j++)                                                         \
        EXPECT_NEAR(A1(i, j), A2(i, j), tol);                                                      \
  }

#define TEST_OP_MAP(opname, operator, operatorf)                                                   \
  TEST(FactorizedRankTwoTensor, opname)                                                            \
  {                                                                                                \
    RankTwoTensor A0(7, 2, 3, 2, 5, 3, 3, 3, 9);                                                   \
    FactorizedRankTwoTensor A(A0);                                                                 \
                                                                                                   \
    std::vector<Real> eigvals;                                                                     \
    RankTwoTensor eigvecs;                                                                         \
    A0.symmetricEigenvaluesEigenvectors(eigvals, eigvecs);                                         \
    for (auto & eigval : eigvals)                                                                  \
      eigval = operator;                                                                           \
    RankTwoTensor op_eigvals(eigvals);                                                             \
    RankTwoTensor op_A = eigvecs * op_eigvals * eigvecs.transpose();                               \
                                                                                                   \
    FactorizedRankTwoTensor op_A_f = operatorf;                                                    \
    EXPECT_R2T_NEAR(op_A, op_A_f.get(), 1e-6);                                                     \
  }                                                                                                \
  static_assert(true, "")

TEST_OP_MAP(log, std::log(eigval), MathUtils::log(A));
TEST_OP_MAP(exp, std::exp(eigval), MathUtils::exp(A));
TEST_OP_MAP(sqrt, std::sqrt(eigval), MathUtils::sqrt(A));
TEST_OP_MAP(cbrt, std::cbrt(eigval), MathUtils::cbrt(A));
TEST_OP_MAP(pow, std::pow(eigval, 3.3), MathUtils::pow(A, 3.3));

TEST(FactorizedRankTwoTensor, constructors)
{
  RankTwoTensor A1(1, 2, 3, 2, 5, -3, 3, -3, -9);
  std::vector<Real> eigvals;
  RankTwoTensor eigvecs;
  A1.symmetricEigenvaluesEigenvectors(eigvals, eigvecs);

  // From a RankTwoTensor
  FactorizedRankTwoTensor A2(A1);
  EXPECT_R2T_NEAR(A1, A2.get(), 1e-6);
  EXPECT_VEC_NEAR(eigvals, A2.eigvals(), 1e-6);
  EXPECT_R2T_NEAR(eigvecs, A2.eigvecs(), 1e-6);

  // From a FactorizedRankTwoTensor
  FactorizedRankTwoTensor A3(A2);
  EXPECT_R2T_NEAR(A1, A3.get(), 1e-6);
  EXPECT_VEC_NEAR(eigvals, A3.eigvals(), 1e-6);
  EXPECT_R2T_NEAR(eigvecs, A3.eigvecs(), 1e-6);

  // From a factorization
  FactorizedRankTwoTensor A4(eigvals, eigvecs);
  EXPECT_R2T_NEAR(A1, A4.get(), 1e-6);
  EXPECT_VEC_NEAR(eigvals, A4.eigvals(), 1e-6);
  EXPECT_R2T_NEAR(eigvecs, A4.eigvecs(), 1e-6);
}

TEST(FactorizedRankTwoTensor, rotated)
{
  RankTwoTensor A(1, 2, 3, 2, 5, -3, 3, -3, -9);
  FactorizedRankTwoTensor Af(A);
  RankTwoTensor R(0, 1, 0, 1, 0, 0, 0, 0, 1);

  // Apply rotation R on A2
  FactorizedRankTwoTensor Afr = Af.rotated(R);
  EXPECT_R2T_NEAR(A.rotated(R), Afr.get(), 1e-6);
}

TEST(FactorizedRankTwoTensor, transpose)
{
  RankTwoTensor A(1, 2, 3, 2, 5, -3, 3, -3, -9);
  FactorizedRankTwoTensor Af(A);

  // Transpose
  FactorizedRankTwoTensor Aft = Af.transpose();
  EXPECT_R2T_NEAR(A.transpose(), Aft.get(), 1e-6);
}

TEST(FactorizedRankTwoTensor, assignment_operator)
{
  RankTwoTensor A(1, 2, 3, 2, 5, -3, 3, -3, -9);
  FactorizedRankTwoTensor Af(A);
  FactorizedRankTwoTensor Bf(A);

  // Assignment from FactorizedRankTwoTensor
  Bf = Af;
  EXPECT_R2T_NEAR(Af.get(), Bf.get(), 1e-6);
  EXPECT_VEC_NEAR(Af.eigvals(), Bf.eigvals(), 1e-6);
  EXPECT_R2T_NEAR(Af.eigvecs(), Bf.eigvecs(), 1e-6);

  // Assignment from RankTwoTensor
  Bf = A;
  EXPECT_R2T_NEAR(Af.get(), Bf.get(), 1e-6);
  EXPECT_VEC_NEAR(Af.eigvals(), Bf.eigvals(), 1e-6);
  EXPECT_R2T_NEAR(Af.eigvecs(), Bf.eigvecs(), 1e-6);
}

TEST(FactorizedRankTwoTensor, scalar_multiplication_division)
{
  RankTwoTensor A(1, 2, 3, 2, 5, -3, 3, -3, -9);
  FactorizedRankTwoTensor Af(A);

  // operator*=
  A *= 2;
  Af *= 2;
  EXPECT_R2T_NEAR(A, Af.get(), 1e-6);

  // operator*
  RankTwoTensor B = A * 2;
  FactorizedRankTwoTensor Bf = Af * 2;
  EXPECT_R2T_NEAR(B, Bf.get(), 1e-6);

  // operator/=
  A /= 3;
  Af /= 3;
  EXPECT_R2T_NEAR(A, Af.get(), 1e-6);

  // operator/
  RankTwoTensor C = A / 2;
  FactorizedRankTwoTensor Cf = Af / 2;
  EXPECT_R2T_NEAR(C, Cf.get(), 1e-6);
}

TEST(FactorizedRankTwoTensor, logical_equal)
{
  RankTwoTensor A(1, 2, 3, 2, 5, -3, 3, -3, -9);
  RankTwoTensor B(1, 3, 3, 3, 5, -1, 3, -1, 4);
  FactorizedRankTwoTensor Af1(A);
  FactorizedRankTwoTensor Af2(A);
  FactorizedRankTwoTensor Bf(B);

  EXPECT_TRUE(Af1 == Af2);
  EXPECT_FALSE(Af1 == Bf);
}

TEST(FactorizedRankTwoTensor, inverse)
{
  RankTwoTensor A(1, 2, 3, 2, 5, -3, 3, -3, -9);
  FactorizedRankTwoTensor Af(A);

  RankTwoTensor Ainv = A.inverse();
  FactorizedRankTwoTensor Afinv = Af.inverse();
  EXPECT_R2T_NEAR(Ainv, Afinv.get(), 1e-6);
}

TEST(FactorizedRankTwoTensor, addIa)
{
  RankTwoTensor A(1, 2, 3, 2, 5, -3, 3, -3, -9);
  FactorizedRankTwoTensor Af(A);

  A.addIa(100);
  Af.addIa(100);
  EXPECT_R2T_NEAR(A, Af.get(), 1e-6);
}
