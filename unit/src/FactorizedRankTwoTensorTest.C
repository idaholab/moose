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
#include "RankFourTensor.h"

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

TEST(FactorizedRankTwoTensor, trace)
{
  RankTwoTensor A(1, 2, 3, 2, 5, -3, 3, -3, -9);
  FactorizedRankTwoTensor Af(A);

  EXPECT_NEAR(A.trace(), Af.trace(), 1e-6);
}

TEST(FactorizedRankTwoTensor, det)
{
  RankTwoTensor A(1, 2, 3, 2, 5, -3, 3, -3, -9);
  FactorizedRankTwoTensor Af(A);

  EXPECT_NEAR(A.det(), Af.det(), 1e-6);
}

TEST(FactorizedRankTwoTensor, dexp)
{
  RankTwoTensor A(2.28, -0.96, 0, -0.96, 1.72, 0, 0, 0, 5);
  FactorizedRankTwoTensor Af(A);

  RankFourTensor deriv = MathUtils::dexp(Af);

  // The derivative obtained with finite differencing
  RankFourTensor deriv_fd(
      {12.580740822443, -4.533478302333, 0.000000000000,  -4.533478302333, 1.252584267197,
       0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,  -4.533478304429,
       5.594398042579,  0.000000000000,  5.594398042579,  -3.802804147881, 0.000000000000,
       0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,
       27.088689036041, 0.000000000000,  0.000000000000,  -6.657622026557, 27.088689036041,
       -6.657622026557, 0.000000000000,  -4.533478304429, 5.594398042579,  0.000000000000,
       5.594398042579,  -3.802804147872, 0.000000000000,  0.000000000000,  0.000000000000,
       0.000000000000,  1.252584267259,  -3.802804146646, 0.000000000000,  -3.802804146646,
       7.717909395932,  0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,
       0.000000000000,  0.000000000000,  -6.657622026693, 0.000000000000,  0.000000000000,
       23.205076186275, -6.657622026693, 23.205076186275, 0.000000000000,  0.000000000000,
       0.000000000000,  27.088689036041, 0.000000000000,  0.000000000000,  -6.657622026557,
       27.088689036041, -6.657622026557, 0.000000000000,  0.000000000000,  0.000000000000,
       -6.657622026693, 0.000000000000,  0.000000000000,  23.205076186275, -6.657622026693,
       23.205076186275, 0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,
       0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,
       148.413159099903},
      RankFourTensor::general);

  EXPECT_NEAR(0, (deriv - deriv_fd).L2norm(), 1E-5);
}

TEST(FactorizedRankTwoTensor, dlog)
{
  RankTwoTensor A(2.28, -0.96, 0, -0.96, 1.72, 0, 0, 0, 5);
  FactorizedRankTwoTensor Af(A);

  RankFourTensor deriv = MathUtils::dlog(Af);

  // The derivative obtained with finite differencing
  RankFourTensor deriv_fd(
      {0.519253605168, 0.144226745964, 0.000000000000, 0.144226745964, 0.054079728859,
       0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.144226746022,
       0.328732801061, 0.000000000000, 0.328732801061, 0.175773254658, 0.000000000000,
       0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.154156805876, 0.000000000000, 0.000000000000, 0.035267199900, 0.154156805876,
       0.035267199900, 0.000000000000, 0.144226746023, 0.328732801061, 0.000000000000,
       0.328732801061, 0.175773254658, 0.000000000000, 0.000000000000, 0.000000000000,
       0.000000000000, 0.054079728793, 0.175773254412, 0.000000000000, 0.175773254412,
       0.705920272609, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.000000000000, 0.000000000000, 0.035267199899, 0.000000000000, 0.000000000000,
       0.174729339154, 0.035267199899, 0.174729339154, 0.000000000000, 0.000000000000,
       0.000000000000, 0.154156805876, 0.000000000000, 0.000000000000, 0.035267199900,
       0.154156805876, 0.035267199900, 0.000000000000, 0.000000000000, 0.000000000000,
       0.035267199899, 0.000000000000, 0.000000000000, 0.174729339154, 0.035267199899,
       0.174729339154, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.200000000027},
      RankFourTensor::general);

  EXPECT_NEAR(0, (deriv - deriv_fd).L2norm(), 1E-5);
}

TEST(FactorizedRankTwoTensor, dsqrt)
{
  RankTwoTensor A(2.28, -0.96, 0, -0.96, 1.72, 0, 0, 0, 5);
  FactorizedRankTwoTensor Af(A);

  RankFourTensor deriv = MathUtils::dsqrt(Af);

  // The derivative obtained with finite differencing
  RankFourTensor deriv_fd(
      {0.351705841319, 0.046912812952, 0.000000000000, 0.046912812952, 0.013046244973,
       0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.046912812964,
       0.196058946879, 0.000000000000, 0.196058946879, 0.054523122556, 0.000000000000,
       0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.136265806181, 0.000000000000, 0.000000000000, 0.013682018260, 0.136265806181,
       0.013682018260, 0.000000000000, 0.046912812964, 0.196058946878, 0.000000000000,
       0.196058946878, 0.054523122557, 0.000000000000, 0.000000000000, 0.000000000000,
       0.000000000000, 0.013046244964, 0.054523122512, 0.000000000000, 0.054523122512,
       0.410876803778, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.000000000000, 0.000000000000, 0.013682018260, 0.000000000000, 0.000000000000,
       0.144246983500, 0.013682018260, 0.144246983500, 0.000000000000, 0.000000000000,
       0.000000000000, 0.136265806181, 0.000000000000, 0.000000000000, 0.013682018260,
       0.136265806181, 0.013682018260, 0.000000000000, 0.000000000000, 0.000000000000,
       0.013682018260, 0.000000000000, 0.000000000000, 0.144246983500, 0.013682018260,
       0.144246983500, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.223606797760},
      RankFourTensor::general);

  EXPECT_NEAR(0, (deriv - deriv_fd).L2norm(), 1E-5);
}

TEST(FactorizedRankTwoTensor, dcbrt)
{
  RankTwoTensor A(2.28, -0.96, 0, -0.96, 1.72, 0, 0, 0, 5);
  FactorizedRankTwoTensor Af(A);

  RankFourTensor deriv = MathUtils::dcbrt(Af);

  // The derivative obtained with finite differencing
  RankFourTensor deriv_fd(
      {0.210732681557, 0.038090385822, 0.000000000000, 0.038090385822, 0.011827288029,
       0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.038090385834,
       0.122389680616, 0.000000000000, 0.122389680616, 0.044989637207, 0.000000000000,
       0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.074785137823, 0.000000000000, 0.000000000000, 0.010471391638, 0.074785137823,
       0.010471391638, 0.000000000000, 0.038090385834, 0.122389680615, 0.000000000000,
       0.122389680615, 0.044989637207, 0.000000000000, 0.000000000000, 0.000000000000,
       0.000000000000, 0.011827288020, 0.044989637163, 0.000000000000, 0.044989637163,
       0.259196028400, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.000000000000, 0.000000000000, 0.010471391637, 0.000000000000, 0.000000000000,
       0.080893449612, 0.010471391637, 0.080893449612, 0.000000000000, 0.000000000000,
       0.000000000000, 0.074785137823, 0.000000000000, 0.000000000000, 0.010471391638,
       0.074785137823, 0.010471391638, 0.000000000000, 0.000000000000, 0.000000000000,
       0.010471391637, 0.000000000000, 0.000000000000, 0.080893449612, 0.010471391637,
       0.080893449612, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000, 0.000000000000,
       0.113998396454},
      RankFourTensor::general);

  EXPECT_NEAR(0, (deriv - deriv_fd).L2norm(), 1E-5);
}

TEST(FactorizedRankTwoTensor, dpow)
{
  RankTwoTensor A(2.28, -0.96, 0, -0.96, 1.72, 0, 0, 0, 5);
  FactorizedRankTwoTensor Af(A);

  RankFourTensor deriv = MathUtils::dpow(Af, 2.0);

  // The derivative obtained with finite differencing
  RankFourTensor deriv_fd(
      {4.560000000007,  -0.960000000014, 0.000000000000,  -0.960000000014, 0.000000000022,
       0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,  -0.960000000008,
       2.000000000006,  0.000000000000,  2.000000000006,  -0.960000000012, 0.000000000000,
       0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,
       3.640000000000,  0.000000000000,  0.000000000000,  -0.480000000000, 3.640000000000,
       -0.480000000000, 0.000000000000,  -0.960000000003, 2.000000000006,  0.000000000000,
       2.000000000006,  -0.960000000012, 0.000000000000,  0.000000000000,  0.000000000000,
       0.000000000000,  0.000000000009,  -0.960000000005, 0.000000000000,  -0.960000000005,
       3.440000000008,  0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,
       0.000000000000,  0.000000000000,  -0.480000000000, 0.000000000000,  0.000000000000,
       3.360000000000,  -0.480000000000, 3.360000000000,  0.000000000000,  0.000000000000,
       0.000000000000,  3.640000000000,  0.000000000000,  0.000000000000,  -0.480000000000,
       3.640000000000,  -0.480000000000, 0.000000000000,  0.000000000000,  0.000000000000,
       -0.480000000000, 0.000000000000,  0.000000000000,  3.360000000000,  -0.480000000000,
       3.360000000000,  0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,
       0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,  0.000000000000,
       9.999999999977},
      RankFourTensor::general);

  EXPECT_NEAR(0, (deriv - deriv_fd).L2norm(), 1E-5);
}
