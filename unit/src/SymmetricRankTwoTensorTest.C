//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SymmetricRankTwoTensorTest.h"
#include "RankTwoTensor.h"
#include "MooseTypes.h"
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

// TEST_F(SymmetricRankTwoTensorTest, doubleContraction)
// {
//   EXPECT_NEAR(121, _m3.doubleContraction(_unsymmetric0), 0.0001);
// }

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

  for (auto i : make_range(3))
    for (auto j : make_range(3))
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
  auto sym1 = RankTwoTensor::initializeSymmetric(
      RealVectorValue(1, 2, 3), RealVectorValue(4, 5, 6), RealVectorValue(7, 8, 9));

  auto sym2 = RankTwoTensor::initializeFromRows(
      RealVectorValue(1, 3, 5), RealVectorValue(3, 5, 7), RealVectorValue(5, 7, 9));

  for (auto i : make_range(3))
    for (auto j : make_range(3))
      EXPECT_EQ(sym1(i, j), sym2(i, j));
}

TEST_F(SymmetricRankTwoTensorTest, rowMultiply)
{
  for (auto i : make_range(3))
    EXPECT_NEAR(_m3.rowMultiply(i, _v), RankTwoTensor(_m3).rowMultiply(i, _v), 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, timesTranspose)
{
  auto A = SymmetricRankTwoTensor::timesTranspose(_m3);
  auto B = SymmetricRankTwoTensor::timesTranspose(RankTwoTensor(_m3));
  auto C = RankTwoTensor::timesTranspose(RankTwoTensor(_m3));
  EXPECT_NEAR((A - B).L2norm(), 0, 1e-9);
  EXPECT_NEAR((RankTwoTensor(A) - C).L2norm(), 0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, plusTranspose)
{
  auto A = SymmetricRankTwoTensor::plusTranspose(_m3);
  auto B = RankTwoTensor::plusTranspose(RankTwoTensor(_m3));
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, sqr)
{
  auto A = _m3.sqr();
  auto B = RankTwoTensor(_m3).sqr();
  EXPECT_NEAR((RankTwoTensor(A) - B).L2norm(), 0, 1e-9);
}

TEST_F(SymmetricRankTwoTensorTest, vectorSelfOuterProduct)
{
  auto A = SymmetricRankTwoTensor::vectorSelfOuterProduct(_v);
  auto B = RankTwoTensor::vectorSelfOuterProduct(_v);
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
