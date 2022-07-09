//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SymmetricRankTwoTensorTest.h"
#include "SymmetricRankFourTensor.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "RotationMatrix.h"
#include "MooseTypes.h"
#include "MooseArray.h"
#include "ADReal.h"

#include "libmesh/point.h"
#include "libmesh/int_range.h"

#include "metaphysicl/raw_type.h"

TEST_F(SymmetricRankTwoTensorTest, L2norm)
{
  EXPECT_NEAR(0, _m0.L2norm(), 1e-5);
  EXPECT_NEAR(1.732051, _m1.L2norm(), 1e-5);
  EXPECT_NEAR(3.741657, _m2.L2norm(), 1e-5);
  EXPECT_NEAR(14.31782, _m3.L2norm(), 1e-5);

  EXPECT_NEAR(RankTwoTensor(_m0).L2norm(), _m0.L2norm(), 1e-5);
  EXPECT_NEAR(RankTwoTensor(_m1).L2norm(), _m1.L2norm(), 1e-5);
  EXPECT_NEAR(RankTwoTensor(_m2).L2norm(), _m2.L2norm(), 1e-5);
  EXPECT_NEAR(RankTwoTensor(_m3).L2norm(), _m3.L2norm(), 1e-5);
}

TEST_F(SymmetricRankTwoTensorTest, initIdentity)
{
  SymmetricRankTwoTensor A(SymmetricRankTwoTensor::InitMethod::initIdentity);
  EXPECT_NEAR(0, (A - _m1).L2norm(), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, addIa)
{
  SymmetricRankTwoTensor m(1, 2, 3, 4, 5, 6);
  m.addIa(0.1);
  SymmetricRankTwoTensor n(1.1, 2.1, 3.1, 4, 5, 6);
  EXPECT_NEAR(0, (m - n).L2norm(), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, transpose)
{
  EXPECT_NEAR(0, (_m0.transpose() - _m0).L2norm(), 1e-9);
  EXPECT_NEAR(0, (_m1.transpose() - _m1).L2norm(), 1e-9);
  EXPECT_NEAR(0, (_m2.transpose() - _m2).L2norm(), 1e-9);
  EXPECT_NEAR(0, (_m3.transpose() - _m3).L2norm(), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, trace)
{
  EXPECT_NEAR(0, _m0.tr(), 1e-9);
  EXPECT_NEAR(3, _m1.tr(), 1e-9);
  EXPECT_NEAR(6, _m2.tr(), 1e-9);
  EXPECT_NEAR(5, _m3.tr(), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, secondInvariant)
{
  EXPECT_NEAR(0, _m0.secondInvariant(), 1e-9);
  EXPECT_NEAR(0, _m1.secondInvariant(), 1e-9);
  EXPECT_NEAR(1, _m2.secondInvariant(), 1e-9);
  EXPECT_NEAR(RankTwoTensor(_m3).secondInvariant(), _m3.secondInvariant(), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, det)
{
  EXPECT_NEAR(0, _m0.det(), 1e-9);
  EXPECT_NEAR(1, _m1.det(), 1e-9);
  EXPECT_NEAR(6, _m2.det(), 1e-9);
  EXPECT_NEAR(-144, _m3.det(), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, deviatoric)
{
  SymmetricRankTwoTensor dev(-1, 0, 1, 0, 0, 0);
  EXPECT_NEAR(0, (dev - _m2.deviatoric()).L2norm(), 1e-9);
  EXPECT_NEAR(0, (_m0 - _m1.deviatoric()).L2norm(), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, inverse)
{
  // the result of the multiplication operator is not guaranteed to be symmetric, so that operator
  // is not implemented for the symmetric tensor class. That's why we need to use an explicit cast
  // to the full tensor before performing the multiplication.
  EXPECT_NEAR(
      0, (RankTwoTensor(_m3) * RankTwoTensor(_m3.inverse()) - RankTwoTensor(_m1)).L2norm(), 1e-9);

  // trying to invert a null matrix
  try
  {
    _m0.inverse();
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Matrix not invertible") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(SymmetricRankTwoTensorTest, dtrace)
{
  EXPECT_NEAR(0, (_m0.dtrace() - _m1).L2norm(), 1e-9);
  EXPECT_NEAR(0, (_m3.dtrace() - _m1).L2norm(), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, ddet)
{
  auto uddet = RankTwoTensor(_m3).ddet();
  auto sddet = RankTwoTensor(_m3.ddet());

  for (const auto i : make_range(3))
    for (const auto j : make_range(3))
      EXPECT_NEAR(uddet(i, j), sddet(i, j), 1e-5);
}

TEST_F(SymmetricRankTwoTensorTest, ADConversion)
{
  SymmetricRankTwoTensor reg;
  ADSymmetricRankTwoTensor ad;

  ad = reg;
  reg = MetaPhysicL::raw_value(ad);

  GenericRankTwoTensor<false> generic_reg;
  GenericRankTwoTensor<true> generic_ad;

  generic_ad = generic_reg;
  generic_reg = MetaPhysicL::raw_value(generic_ad);
}

TEST_F(SymmetricRankTwoTensorTest, initializeSymmetric)
{
  auto A = SymmetricRankTwoTensor::initializeSymmetric(
      RealVectorValue(1, 2, 3), RealVectorValue(4, 5, 6), RealVectorValue(7, 8, 9));
  auto B = RankTwoTensor::initializeSymmetric(
      RealVectorValue(1, 2, 3), RealVectorValue(4, 5, 6), RealVectorValue(7, 8, 9));
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, row)
{
  for (auto i : make_range(3))
    EXPECT_NEAR(_m3.row(i) * _v, RankTwoTensor(_m3).row(i) * _v, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, timesTranspose)
{
  auto A = SymmetricRankTwoTensor::timesTranspose(_m3);
  auto B = SymmetricRankTwoTensor::timesTranspose(RankTwoTensor(_m3));
  auto C = RankTwoTensor::timesTranspose(RankTwoTensor(_m3));
  EXPECT_NEAR((A - B).L2norm(), 0, 1e-9);
  EXPECT_NEAR((RankTwoTensor(A) - C).L2norm(), 0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, transposeTimes)
{
  auto A = SymmetricRankTwoTensor::transposeTimes(_m3);
  auto B = SymmetricRankTwoTensor::transposeTimes(RankTwoTensor(_m3));
  auto C = RankTwoTensor::transposeTimes(RankTwoTensor(_m3));
  EXPECT_NEAR((A - B).L2norm(), 0, 1e-9);
  EXPECT_NEAR((RankTwoTensor(A) - C).L2norm(), 0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, plusTranspose)
{
  auto A = SymmetricRankTwoTensor::plusTranspose(_m3);
  auto B = RankTwoTensor::plusTranspose(RankTwoTensor(_m3));
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, square)
{
  auto A = _m3.square();
  auto B = RankTwoTensor(_m3).square();
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, selfOuterProduct)
{
  auto A = SymmetricRankTwoTensor::selfOuterProduct(_v);
  auto B = RankTwoTensor::selfOuterProduct(_v);
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, operators)
{
  const auto f3 = RankTwoTensor(_m3);
  const auto f4 = RankTwoTensor(_m4);

  // multiply by scalar
  {
    auto A = _m3 * 3.31;
    auto B = f3 * 3.31;
    EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // in-place multiply by scalar
  {
    auto A = _m3;
    A *= 3.45;
    auto B = f3;
    B *= 3.45;
    EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // divide by scalar
  {
    auto A = _m3 / 3.31;
    auto B = f3 / 3.31;
    EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // in-place divide by scalar
  {
    auto A = _m3;
    A /= 3.45;
    auto B = f3;
    B /= 3.45;
    EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // addition
  {
    auto A = _m3 + _m4;
    auto B = f3 + f4;
    EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // in-place addition
  {
    auto A = _m3;
    A += _m4;
    auto B = f3;
    B += f4;
    EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // subtraction
  {
    auto A = _m3 - _m4;
    auto B = f3 - f4;
    EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // in-place subtraction
  {
    auto A = _m3;
    A -= _m4;
    auto B = f3;
    B -= f4;
    EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
  }

  // negate
  {
    auto A = -_m3;
    auto B = -f3;
    EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
  }
}

TEST_F(SymmetricRankTwoTensorTest, rotate)
{
  auto R = RotationMatrix::rotVecToZ(RealVectorValue(2, -5, 1));

  auto A = _m3;
  A.rotate(R);
  auto B = RankTwoTensor(_m3);
  B.rotate(R);

  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, fillFromInputVector)
{
  SymmetricRankTwoTensor A;
  RankTwoTensor B;

  A.fillFromInputVector({3.9});
  B.fillFromInputVector({3.9});
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);

  A.fillFromInputVector({3.9, -2.3, 4.8});
  B.fillFromInputVector({3.9, -2.3, 4.8});
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);

  A.fillFromInputVector({9.2, 2.7, 4.3, 8.2, 1.4, 3.6});
  B.fillFromInputVector({9.2, 2.7, 4.3, 8.2, 1.4, 3.6});
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);

  try
  {
    A.fillFromInputVector({1, 2});
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Please check the number of entries in the input vector for building a "
                         "SymmetricRankTwoTensorTempl. It must be 1, 3, 6") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(SymmetricRankTwoTensorTest, zero)
{
  auto A = _m4;
  A.zero();
  EXPECT_NEAR(A.L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, doubleContraction)
{
  auto A = RankTwoTensor(_m3);
  auto B = RankTwoTensor(_m4);

  EXPECT_NEAR(A.doubleContraction(B), _m3.doubleContraction(_m4), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, outerProduct)
{
  auto A = RankTwoTensor(_m3);
  auto B = RankTwoTensor(_m4);

  auto p = A.outerProduct(B);
  auto q = _m3.outerProduct(_m4);

  EXPECT_NEAR((RankFourTensor(q) - p).L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, generalSecondInvariant)
{
  EXPECT_NEAR(RankTwoTensor(_m3).generalSecondInvariant(), _m3.generalSecondInvariant(), 1e-9);
  EXPECT_NEAR(RankTwoTensor(_m4).generalSecondInvariant(), _m4.generalSecondInvariant(), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, dsecondInvariant)
{
  auto A = _m3.dsecondInvariant();
  auto B = RankTwoTensor(_m3).dsecondInvariant();
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, d2secondInvariant)
{
  auto A = _m3.d2secondInvariant();
  auto B = RankTwoTensor(_m3).d2secondInvariant();
  EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, thirdInvariant)
{
  EXPECT_NEAR(RankTwoTensor(_m3).thirdInvariant(), _m3.thirdInvariant(), 1e-9);
  EXPECT_NEAR(RankTwoTensor(_m4).thirdInvariant(), _m4.thirdInvariant(), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, dthirdInvariant)
{
  auto A = _m3.dthirdInvariant();
  auto B = RankTwoTensor(_m3).dthirdInvariant();
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, d2thirdInvariant)
{
  auto A = _m3.d2thirdInvariant();
  auto B = RankTwoTensor(_m3).d2thirdInvariant();
  EXPECT_NEAR((RankFourTensor(A) - B).L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, fullAccessOperator)
{
  auto A = _m3;
  auto B = RankTwoTensor(_m3);

  for (const auto i : make_range(3))
    for (const auto j : make_range(3))
      EXPECT_NEAR(A(i, j), B(i, j), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, positiveProjectionEigenDecomposition)
{
  std::vector<Real> Aeigval;
  RankTwoTensorTempl<Real> Aeigvec;
  auto A = RankFourTensor(_m3.positiveProjectionEigenDecomposition(Aeigval, Aeigvec));

  std::vector<Real> Beigval;
  RankTwoTensorTempl<Real> Beigvec;
  auto B = RankTwoTensor(_m3).positiveProjectionEigenDecomposition(Beigval, Beigvec);

  EXPECT_NEAR((Beigvec - Aeigvec).L2norm(), 0.0, 1e-9);
  EXPECT_NEAR(Aeigval[0], Beigval[0], 1e-9);
  EXPECT_NEAR(Aeigval[1], Beigval[1], 1e-9);
  EXPECT_NEAR(Aeigval[2], Beigval[2], 1e-9);
  EXPECT_NEAR((A - B).L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, print)
{
  std::stringstream ss;
  _m3.print(ss);
  const std::string gold = "              1\n             -5\n              9\n       -8.48528\n   "
                           "     4.24264\n        2.82843\n";
  EXPECT_EQ(ss.str(), gold);
}

TEST_F(SymmetricRankTwoTensorTest, printReal)
{
  std::stringstream ss;
  _m3.printReal(ss);
  const std::string gold = "              1\n             -5\n              9\n       -8.48528\n   "
                           "     4.24264\n        2.82843\n";
  EXPECT_EQ(ss.str(), gold);
}

TEST_F(SymmetricRankTwoTensorTest, symmetricEigenvaluesEigenvectors)
{
  std::vector<Real> A1, C1;
  RankTwoTensor A2;
  _m3.symmetricEigenvaluesEigenvectors(A1, A2);
  RankTwoTensor(_m3).symmetricEigenvalues(C1);

  std::vector<Real> B1, D1;
  RankTwoTensor B2;
  RankTwoTensor(_m3).symmetricEigenvaluesEigenvectors(B1, B2);
  RankTwoTensor(_m3).symmetricEigenvalues(D1);

  EXPECT_EQ(A1.size(), B1.size());

  for (auto i : index_range(A1))
  {
    EXPECT_NEAR(A1[i], B1[i], 1e-9);
    EXPECT_NEAR(C1[i], D1[i], 1e-9);
  }
  EXPECT_NEAR((A2 - B2).L2norm(), 0, 1e-9);

  try
  {
    ADSymmetricRankTwoTensor A;
    std::vector<ADReal> v;
    ADRankTwoTensor B;
    A.symmetricEigenvaluesEigenvectors(v, B);
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("symmetricEigenvaluesEigenvectors is only available for ordered tensor "
                         "component types") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(SymmetricRankTwoTensorTest, genRandomSymmTensor)
{
  SymmetricRankTwoTensor::initRandom(12345);
  auto A = SymmetricRankTwoTensor::genRandomSymmTensor(2.2, 7);
  RankTwoTensor::initRandom(12345);
  auto B = RankTwoTensor::genRandomSymmTensor(2.2, 7);
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, initialContraction)
{
  SymmetricRankFourTensor F;
  for (auto i : make_range(6))
    for (auto j : make_range(6))
      F(i, j) = (i + 1) * 10 + j + 1;
  RankFourTensor G = RankFourTensor(F);

  SymmetricRankTwoTensor A = _m4;
  RankTwoTensor B = RankTwoTensor(_m4);

  A.initialContraction(F);
  B.initialContraction(G);

  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0.0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, fillFromScalarVariable)
{
  SymmetricRankTwoTensor A;

  SymmetricRankTwoTensor A1(1, 0, 0, 0, 0, 0);
  VariableValue s1(1);
  s1[0] = 1;
  A.fillFromScalarVariable(s1);
  EXPECT_NEAR((A1 - A).L2norm(), 0.0, 1e-9);

  SymmetricRankTwoTensor A2(1, 2, 0, 0, 0, 3);
  VariableValue s2(3);
  s2[0] = 1;
  s2[1] = 2;
  s2[2] = 3;
  A.fillFromScalarVariable(s2);
  EXPECT_NEAR((A2 - A).L2norm(), 0.0, 1e-9);

  SymmetricRankTwoTensor A3(1, 2, 3, 4, 5, 6);
  VariableValue s3(6);
  s3[0] = 1;
  s3[1] = 2;
  s3[2] = 3;
  s3[3] = 4;
  s3[4] = 5;
  s3[5] = 6;
  A.fillFromScalarVariable(s3);
  EXPECT_NEAR((A3 - A).L2norm(), 0.0, 1e-9);

  VariableValue s4(2);
  try
  {
    A.fillFromScalarVariable(s4);
    FAIL() << "missing expected exception";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Only FIRST, THIRD, or SIXTH order scalar variable can be used to build "
                         "a SymmetricRankTwoTensorTempl.") != std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST_F(SymmetricRankTwoTensorTest, sin3Lode)
{
  // secondInvariant of _m3 is  98.33333

  const auto s = _m3.sin3Lode(200, 999);
  EXPECT_NEAR(s, 999, 1e-9);

  const auto sA = _m3.sin3Lode(0, 999);
  const auto sB = RankTwoTensor(_m3).sin3Lode(0, 999);
  EXPECT_NEAR(sA, sB, 1e-9);

  const auto dA = RankTwoTensor(_m3.dsin3Lode(0));
  const auto dB = RankTwoTensor(_m3).dsin3Lode(0);
  EXPECT_NEAR((dA - dB).L2norm(), 0.0, 1e-9);
}
