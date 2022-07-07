//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SymmetricRankFourTensorTest.h"
#include "SymmetricRankFourTensorImplementation.h"
#include "SymmetricRankTwoTensor.h"
#include "RankTwoTensor.h"
#include "RotationMatrix.h"
#include "MooseTypes.h"
#include "ADReal.h"

#include "libmesh/int_range.h"

#include "metaphysicl/raw_type.h"
#include "metaphysicl/dualnumberarray.h"

TEST_F(SymmetricRankFourTensorTest, ADConversion)
{
  SymmetricRankFourTensor reg;
  ADSymmetricRankFourTensor ad;

  ad = reg;
  reg = MetaPhysicL::raw_value(ad);

  GenericRankFourTensor<false> generic_reg;
  GenericRankFourTensor<true> generic_ad;

  generic_ad = generic_reg;
  generic_reg = MetaPhysicL::raw_value(generic_ad);
}

TEST_F(SymmetricRankFourTensorTest, symmetric9)
{
  RankFourTensor A(_i1, RankFourTensor::symmetric9);
  SymmetricRankFourTensor B(_i1, SymmetricRankFourTensor::symmetric9);
  EXPECT_NEAR(0, (A - RankFourTensor(B)).L2norm(), 1E-5);
}

TEST_F(SymmetricRankFourTensorTest, symmetric21)
{
  RankFourTensor A(_i2, RankFourTensor::symmetric21);
  SymmetricRankFourTensor B(_i2, SymmetricRankFourTensor::symmetric21);
  EXPECT_NEAR(0, (A - RankFourTensor(B)).L2norm(), 1E-9);
}

TEST_F(SymmetricRankFourTensorTest, symmetric_isotropic)
{
  RankFourTensor A({0.37, 4.8}, RankFourTensor::symmetric_isotropic);
  SymmetricRankFourTensor B({0.37, 4.8}, SymmetricRankFourTensor::symmetric_isotropic);
  EXPECT_NEAR(0, (A - RankFourTensor(B)).L2norm(), 1E-9);
  EXPECT_EQ(A.isIsotropic(), true);
  EXPECT_EQ(B.isIsotropic(), true);
}

TEST_F(SymmetricRankFourTensorTest, symmetric_isotropic_E_nu)
{
  RankFourTensor A({1234, 0.7}, RankFourTensor::symmetric_isotropic_E_nu);
  SymmetricRankFourTensor B({1234, 0.7}, SymmetricRankFourTensor::symmetric_isotropic_E_nu);
  EXPECT_NEAR(0, (A - RankFourTensor(B)).L2norm(), 1E-9);
}

TEST_F(SymmetricRankFourTensorTest, axisymmetric_rz)
{
  RankFourTensor A({1, -20, 33.7, 4.4, -55}, RankFourTensor::axisymmetric_rz);
  SymmetricRankFourTensor B({1, -20, 33.7, 4.4, -55}, SymmetricRankFourTensor::axisymmetric_rz);
  EXPECT_NEAR(0, (A - RankFourTensor(B)).L2norm(), 1E-9);
}

TEST_F(SymmetricRankFourTensorTest, principal)
{
  RankFourTensor A({1, -20, 33.7, 4.4, -55, -6.66669, 700, -88.8, 0.9}, RankFourTensor::principal);
  SymmetricRankFourTensor B({1, -20, 33.7, 4.4, -55, -6.66669, 700, -88.8, 0.9},
                            SymmetricRankFourTensor::principal);
  EXPECT_NEAR(0, (A - RankFourTensor(B)).L2norm(), 1E-9);
}

TEST_F(SymmetricRankFourTensorTest, orthotropic)
{
  RankFourTensor A({2, 3, 4, 1.1, 2.2, 3.3, 0.3, 0.25, 0.1, 0.2, 0.125, 0.075},
                   RankFourTensor::orthotropic);
  SymmetricRankFourTensor B({2, 3, 4, 1.1, 2.2, 3.3, 0.3, 0.25, 0.1, 0.2, 0.125, 0.075},
                            SymmetricRankFourTensor::orthotropic);
  EXPECT_NEAR(0, (A - RankFourTensor(B)).L2norm(), 1E-9);
}

TEST_F(SymmetricRankFourTensorTest, initIdentity)
{
  RankFourTensor A(RankFourTensor::InitMethod::initIdentity);
  SymmetricRankFourTensor B(SymmetricRankFourTensor::InitMethod::initIdentity);
  EXPECT_NEAR(0, (A - RankFourTensor(B)).L2norm(), 1E-9);
}

TEST_F(SymmetricRankFourTensorTest, initIdentitySymmetricFour)
{
  RankFourTensor A(RankFourTensor::InitMethod::initIdentitySymmetricFour);
  SymmetricRankFourTensor B(SymmetricRankFourTensor::InitMethod::initIdentitySymmetricFour);
  EXPECT_NEAR(0, (A - RankFourTensor(B)).L2norm(), 1E-9);
}

TEST_F(SymmetricRankFourTensorTest, print)
{
  SymmetricRankFourTensor a;
  for (auto i : make_range(6))
    for (auto j : make_range(6))
      a(i, j) = 10 * i + j;

  std::stringstream ss;
  a.print(ss);
  std::string gold = "              0               1               2               3              "
                     " 4               5 \n"
                     "             10              11              12              13              "
                     "14              15 \n"
                     "             20              21              22              23              "
                     "24              25 \n"
                     "             30              31              32              33              "
                     "34              35 \n"
                     "             40              41              42              43              "
                     "44              45 \n"
                     "             50              51              52              53              "
                     "54              55 \n";
  EXPECT_EQ(ss.str(), gold);
}

TEST_F(SymmetricRankFourTensorTest, printReal)
{
  std::stringstream ss;
  _s2.printReal(ss);
  const std::string gold = "              1               2               3         5.65685        "
                           " 7.07107         8.48528 \n"
                           "              2               7               8         12.7279        "
                           " 14.1421         15.5563 \n"
                           "              3               8              12         18.3848        "
                           "  19.799         21.2132 \n"
                           "        5.65685         12.7279         18.3848              32        "
                           "      34              36 \n"
                           "        7.07107         14.1421          19.799              34        "
                           "      38              40 \n"
                           "        8.48528         15.5563         21.2132              36        "
                           "      40              42 \n";
  EXPECT_EQ(ss.str(), gold);
}

TEST_F(SymmetricRankFourTensorTest, L2norm)
{
  EXPECT_NEAR(RankFourTensor(_s1).L2norm(), _s1.L2norm(), 1E-5);
  EXPECT_NEAR(RankFourTensor(_s2).L2norm(), _s2.L2norm(), 1E-5);
  EXPECT_NEAR(RankFourTensor(_s3).L2norm(), _s3.L2norm(), 1E-5);
}

TEST_F(SymmetricRankFourTensorTest, r4r2multiply)
{
  auto b = RankFourTensor(_s3);
  SymmetricRankTwoTensor c;
  for (auto i : make_range(6))
    c(i) = 0.1 * (i + 1);
  auto d = RankTwoTensor(c);

  auto p1 = RankTwoTensor(_s3 * c);
  auto p2 = b * d;
  EXPECT_NEAR(0, (p1 - p2).L2norm(), 1E-9);
}

TEST_F(SymmetricRankFourTensorTest, operators)
{
  const auto f3 = RankFourTensor(_s3);
  const auto f2 = RankFourTensor(_s2);

  {
    auto A = _s3 * _s2;
    auto B = f3 * f2;
    EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // multiply by scalar
  {
    auto A = _s3 * 3.31;
    auto B = f3 * 3.31;
    EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // in-place multiply by scalar
  {
    auto A = _s3;
    A *= 3.45;
    auto B = f3;
    B *= 3.45;
    EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // divide by scalar
  {
    auto A = _s3 / 3.31;
    auto B = f3 / 3.31;
    EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // in-place divide by scalar
  {
    auto A = _s3;
    A /= 3.45;
    auto B = f3;
    B /= 3.45;
    EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // addition
  {
    auto A = _s3 + _s2;
    auto B = f3 + f2;
    EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // in-place addition
  {
    auto A = _s3;
    A += _s2;
    auto B = f3;
    B += f2;
    EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // subtraction
  {
    auto A = _s3 - _s2;
    auto B = f3 - f2;
    EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // in-place subtraction
  {
    auto A = _s3;
    A -= _s2;
    auto B = f3;
    B -= f2;
    EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // negate
  {
    auto A = -_s3;
    auto B = -f3;
    EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
  }
}

TEST_F(SymmetricRankFourTensorTest, sum3)
{
  auto f3 = RankFourTensor(_s3);
  EXPECT_NEAR(f3.sum3x3(), _s3.sum3x3(), 1e-9);

  const auto v1 = f3.sum3x1();
  const auto v2 = _s3.sum3x1();

  for (auto i : make_range(3))
    EXPECT_NEAR(v1(i), v2(i), 1e-9);
}

TEST_F(SymmetricRankFourTensorTest, isSymmetric)
{
  EXPECT_EQ(_s1.isSymmetric(), true);
  EXPECT_EQ(_s2.isSymmetric(), true);
  EXPECT_EQ(_s3.isSymmetric(), false);
}

TEST_F(SymmetricRankFourTensorTest, rotate)
{
  auto R = RotationMatrix::rotVecToZ(RealVectorValue(2, -5, 1));

  auto A = _s3;
  A.rotate(R);
  auto B = RankFourTensor(_s3);
  B.rotate(R);

  EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankFourTensorTest, invSym)
{
  auto A = _s2.invSymm();
  auto B = RankFourTensor(_s2).invSymm();
  EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankFourTensorTest, bignum)
{
  constexpr std::size_t derivative_size = 1000;
  typedef NumberArray<derivative_size, Real> DNDerivativeType;
  typedef DualNumber<Real, DNDerivativeType, /*allow_skiping_derivatives=*/true> ADBig;

  SymmetricRankFourTensorTempl<ADBig> A = _s2;
  SymmetricRankFourTensorTempl<ADReal> B = _s2;
  SymmetricRankFourTensorTempl<Real> C = _s2;

  const auto iA = MetaPhysicL::raw_value(A.invSymm());
  const auto iB = MetaPhysicL::raw_value(B.invSymm());
  const auto iC = MetaPhysicL::raw_value(C.invSymm());

  EXPECT_NEAR(MetaPhysicL::raw_value((iA - iB).L2norm()), 0.0, 1e-9);
  EXPECT_NEAR(MetaPhysicL::raw_value((iB - iC).L2norm()), 0.0, 1e-9);
}
