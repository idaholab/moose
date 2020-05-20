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

#include "RankTwoTensor.h"
#include "RankThreeTensor.h"
#include "RankFourTensor.h"
#include "MooseEnum.h"
#include "MooseTypes.h"
#include "ADReal.h"

#include "metaphysicl/raw_type.h"

TEST(RankThreeTensor, constructors)
{
  // Default
  RankThreeTensor r1;
  RankThreeTensor r2(RankThreeTensor::initNone);
  RankThreeTensor r3(std::vector<Real>(27, 1980));
  RankThreeTensor r4(r3);
  RankThreeTensor r5(std::vector<Real>(3, 2013));

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
      {
        EXPECT_EQ(r1(i, j, k), 0);
        EXPECT_EQ(r3(i, j, k), 1980.);
        EXPECT_EQ(r4(i, j, k), 1980.);
      }

  // See fillFromPlaneNormal test for complete test
  EXPECT_FLOAT_EQ(r5(0, 0, 2) / 1e9, -8.1570148);
  EXPECT_FLOAT_EQ(r5(1, 1, 1) / 1e9, -8.1570139);
  EXPECT_FLOAT_EQ(r5(2, 2, 0) / 1e9, -8.1570148);

  try
  {
    RankThreeTensor r6(std::vector<Real>(42));
    FAIL() << "Incorrect error message.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("Unsupported automatic fill method, use 27 values for 'general' and 3 for "
                       "'plane_normal', the supplied size was 42."),
              std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    RankThreeTensor r7(std::vector<Real>(42), RankThreeTensor::general);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(msg.find("To use fillGeneralFromInputVector, your input must have size 27, the "
                       "supplied size was 42."),
              std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    RankThreeTensor r8(std::vector<Real>(42), RankThreeTensor::plane_normal);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_NE(
        msg.find(
            "To use fillFromPlaneNormal, your input must have size 3, the supplied size was 42."),
        std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST(RankThreeTensor, index)
{
  RankThreeTensor r1(std::vector<Real>(27, 2011));
  const RankThreeTensor r2(std::vector<Real>(27, 2013)); //

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
      {
        EXPECT_EQ(r1(i, j, k), 2011);
        EXPECT_EQ(r2(i, j, k), 2013);
      }
}

TEST(RankThreeTensor, zero)
{
  RankThreeTensor r1(std::vector<Real>(27, 1980));
  RankThreeTensor r2(std::vector<Real>(27, 1980));

  r1.zero();
  MathUtils::mooseSetToZero(r2);

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
      {
        EXPECT_EQ(r1(i, j, k), 0);
        EXPECT_EQ(r2(i, j, k), 0);
      }
}

TEST(RankThreeTensor, assignment) // operator=
{
  RankThreeTensor r1, r2;
  r1 = 1980;
  r2 = r1;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
      {
        EXPECT_EQ(r1(i, j, k), 1980);
        EXPECT_EQ(r2(i, j, k), 1980);
      }
}

TEST(RankThreeTensor, multiply)
{
  RankThreeTensor r1(std::vector<Real>(27, 999));
  RankThreeTensor r2 = r1 * 2;
  r1 *= 2;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
      {
        EXPECT_EQ(r1(i, j, k), 1998);
        EXPECT_EQ(r2(i, j, k), 1998);
      }

  // b_i = r_ijk * a_jk
  RankTwoTensor a1(std::vector<Real>(9, 2));
  VectorValue<Real> b = r1 * a1;
  for (unsigned int i = 0; i < 3; ++i)
  {

    Real value = 0;
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        value += r1(i, j, k) * a1(j, k);

    EXPECT_EQ(b(i), value);
  }

  // Stand alone free function operator* overloads
  Real two = 2;
  RankThreeTensor r3 = two * r1;
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        EXPECT_EQ(r3(i, j, k), 3996);

  VectorValue<Real> p(1, 2, 3);
  RankTwoTensor c = p * r1;
  EXPECT_EQ(c(0, 0), p(0) * r1(0, 0, 0) + p(1) * r1(1, 0, 0) + p(2) * r1(2, 0, 0));
  EXPECT_EQ(c(1, 0), p(0) * r1(0, 1, 0) + p(1) * r1(1, 1, 0) + p(2) * r1(2, 1, 0));
  EXPECT_EQ(c(2, 0), p(0) * r1(0, 2, 0) + p(1) * r1(1, 2, 0) + p(2) * r1(2, 2, 0));

  EXPECT_EQ(c(0, 1), p(0) * r1(0, 0, 1) + p(1) * r1(1, 0, 1) + p(2) * r1(2, 0, 1));
  EXPECT_EQ(c(1, 1), p(0) * r1(0, 1, 1) + p(1) * r1(1, 1, 1) + p(2) * r1(2, 1, 1));
  EXPECT_EQ(c(2, 1), p(0) * r1(0, 2, 1) + p(1) * r1(1, 2, 1) + p(2) * r1(2, 2, 1));

  EXPECT_EQ(c(0, 2), p(0) * r1(0, 0, 2) + p(1) * r1(1, 0, 2) + p(2) * r1(2, 0, 2));
  EXPECT_EQ(c(1, 2), p(0) * r1(0, 1, 2) + p(1) * r1(1, 1, 2) + p(2) * r1(2, 1, 2));
  EXPECT_EQ(c(2, 2), p(0) * r1(0, 2, 2) + p(1) * r1(1, 2, 2) + p(2) * r1(2, 2, 2));
}

TEST(RankThreeTensor, divide)
{
  RankThreeTensor r1(std::vector<Real>(27, 1998));
  RankThreeTensor r2 = r1 / 2;
  r1 /= 2;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
      {
        EXPECT_EQ(r1(i, j, k), 999);
        EXPECT_EQ(r2(i, j, k), 999);
      }
}

TEST(RankThreeTensor, plus)
{
  RankThreeTensor r1(std::vector<Real>(27, 999));
  RankThreeTensor r2 = r1 + r1;
  r1 += r1;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
      {
        EXPECT_EQ(r1(i, j, k), 1998);
        EXPECT_EQ(r2(i, j, k), 1998);
      }
}

TEST(RankThreeTensor, minus)
{
  RankThreeTensor r1(std::vector<Real>(27, 1998));
  RankThreeTensor r2(std::vector<Real>(27, 999));
  RankThreeTensor r3 = r1 - r2;
  r1 -= r2;

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
      {
        EXPECT_EQ(r2(i, j, k), 999);
        EXPECT_EQ(r3(i, j, k), 999);
      }
}

TEST(RankThreeTensor, negative)
{
  RankThreeTensor r1(std::vector<Real>(27, 2013));
  RankThreeTensor r2 = -r1;
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        EXPECT_EQ(r2(i, j, k), -2013);
}

TEST(RankThreeTensor, print)
{
  std::ostringstream oss;
  RankThreeTensor r1(std::vector<Real>(27, 1949));
  r1.print(oss);
  std::string msg = oss.str();
  EXPECT_NE(msg.find("a(0, j, k) ="), std::string::npos);
  EXPECT_NE(msg.find("a(1, j, k) ="), std::string::npos);
  EXPECT_NE(msg.find("a(2, j, k) ="), std::string::npos);
}

TEST(RankThreeTensor, L2norm)
{
  Real sum_of_squares = 0;
  RankThreeTensor r1(std::vector<Real>(27, 1949));
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        sum_of_squares += r1(i, j, k) * r1(i, j, k);
  EXPECT_EQ(r1.L2norm(), std::sqrt(sum_of_squares));
}

TEST(RankThreeTensor, rotate)
{
  RankThreeTensor r1(std::vector<Real>(27, 1980));
  TensorValue<Real> x(0, 1, 2, 3, 4, 5, 6, 7, 8);
  r1.rotate(x);

  EXPECT_FLOAT_EQ(r1(0, 0, 0), 53460);
  EXPECT_FLOAT_EQ(r1(0, 1, 0), 213840);
  EXPECT_FLOAT_EQ(r1(0, 2, 0), 374220);
  EXPECT_FLOAT_EQ(r1(0, 0, 1), 213840);
  EXPECT_FLOAT_EQ(r1(0, 1, 1), 855360);
  EXPECT_FLOAT_EQ(r1(0, 2, 1), 1496880);
  EXPECT_FLOAT_EQ(r1(0, 0, 2), 374220);
  EXPECT_FLOAT_EQ(r1(0, 1, 2), 1496880);
  EXPECT_FLOAT_EQ(r1(0, 2, 2), 2619540);

  EXPECT_FLOAT_EQ(r1(1, 0, 0), 213840);
  EXPECT_FLOAT_EQ(r1(1, 1, 0), 855360);
  EXPECT_FLOAT_EQ(r1(1, 2, 0), 1496880);
  EXPECT_FLOAT_EQ(r1(1, 0, 1), 855360);
  EXPECT_FLOAT_EQ(r1(1, 1, 1), 3421440);
  EXPECT_FLOAT_EQ(r1(1, 2, 1), 5987520);
  EXPECT_FLOAT_EQ(r1(1, 0, 2), 1496880);
  EXPECT_FLOAT_EQ(r1(1, 1, 2), 5987520);
  EXPECT_FLOAT_EQ(r1(1, 2, 2), 10478160);

  EXPECT_FLOAT_EQ(r1(2, 0, 0), 374220);
  EXPECT_FLOAT_EQ(r1(2, 1, 0), 1496880);
  EXPECT_FLOAT_EQ(r1(2, 2, 0), 2619540);
  EXPECT_FLOAT_EQ(r1(2, 0, 1), 1496880);
  EXPECT_FLOAT_EQ(r1(2, 1, 1), 5987520);
  EXPECT_FLOAT_EQ(r1(2, 2, 1), 10478160);
  EXPECT_FLOAT_EQ(r1(2, 0, 2), 2619540);
  EXPECT_FLOAT_EQ(r1(2, 1, 2), 10478160);
  EXPECT_FLOAT_EQ(r1(2, 2, 2), 18336780);

  RankThreeTensor r2(std::vector<Real>(27, 1980));
  RankTwoTensor y(std::vector<Real>({0, 1, 2, 3, 4, 5, 6, 7, 8}));
  r2.rotate(y);

  EXPECT_FLOAT_EQ(r2(0, 0, 0), 1443420);
  EXPECT_FLOAT_EQ(r2(0, 1, 0), 1924560);
  EXPECT_FLOAT_EQ(r2(0, 2, 0), 2405700);
  EXPECT_FLOAT_EQ(r2(0, 0, 1), 1924560);
  EXPECT_FLOAT_EQ(r2(0, 1, 1), 2566080);
  EXPECT_FLOAT_EQ(r2(0, 2, 1), 3207600);
  EXPECT_FLOAT_EQ(r2(0, 0, 2), 2405700);
  EXPECT_FLOAT_EQ(r2(0, 1, 2), 3207600);
  EXPECT_FLOAT_EQ(r2(0, 2, 2), 4009500);
}

TEST(RankThreeTensor, fillMethodEnum)
{
  RankThreeTensor r1;
  MooseEnum methods = r1.fillMethodEnum();
  EXPECT_FALSE(methods.isValid());
  EXPECT_EQ(methods.getRawNames(), "general plane_normal");
}

TEST(RankThreeTensor, fillFromInputVector)
{
  RankThreeTensor r1;
  r1.fillFromInputVector(std::vector<Real>(27, 1954));
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        EXPECT_EQ(r1(i, j, k), 1954);
}

TEST(RankThreeTensor, fillFromPlaneNormal)
{
  RankThreeTensor r1;
  r1.fillFromPlaneNormal(VectorValue<Real>(1, 2, 3));

  EXPECT_FLOAT_EQ(r1(0, 0, 0), 0);
  EXPECT_FLOAT_EQ(r1(0, 1, 0), -1);
  EXPECT_FLOAT_EQ(r1(0, 2, 0), -1.5);
  EXPECT_FLOAT_EQ(r1(0, 0, 1), -1);
  EXPECT_FLOAT_EQ(r1(0, 1, 1), -4);
  EXPECT_FLOAT_EQ(r1(0, 2, 1), -6);
  EXPECT_FLOAT_EQ(r1(0, 0, 2), -1.5);
  EXPECT_FLOAT_EQ(r1(0, 1, 2), -6);
  EXPECT_FLOAT_EQ(r1(0, 2, 2), -9);

  EXPECT_FLOAT_EQ(r1(1, 0, 0), -2);
  EXPECT_FLOAT_EQ(r1(1, 1, 0), -3.5);
  EXPECT_FLOAT_EQ(r1(1, 2, 0), -6);
  EXPECT_FLOAT_EQ(r1(1, 0, 1), -3.5);
  EXPECT_FLOAT_EQ(r1(1, 1, 1), -6);
  EXPECT_FLOAT_EQ(r1(1, 2, 1), -10.5);
  EXPECT_FLOAT_EQ(r1(1, 0, 2), -6);
  EXPECT_FLOAT_EQ(r1(1, 1, 2), -10.5);
  EXPECT_FLOAT_EQ(r1(1, 2, 2), -18);

  EXPECT_FLOAT_EQ(r1(2, 0, 0), -3);
  EXPECT_FLOAT_EQ(r1(2, 1, 0), -6);
  EXPECT_FLOAT_EQ(r1(2, 2, 0), -8.5);
  EXPECT_FLOAT_EQ(r1(2, 0, 1), -6);
  EXPECT_FLOAT_EQ(r1(2, 1, 1), -12);
  EXPECT_FLOAT_EQ(r1(2, 2, 1), -17);
  EXPECT_FLOAT_EQ(r1(2, 0, 2), -8.5);
  EXPECT_FLOAT_EQ(r1(2, 1, 2), -17);
  EXPECT_FLOAT_EQ(r1(2, 2, 2), -24);
}

TEST(RankThreeTensor, mixedProductRankFour)
{
  RankThreeTensor A(std::vector<Real>(27, 1949));
  RankTwoTensor b(std::vector<Real>(9, 1954));
  RankFourTensor D = A.mixedProductRankFour(b);

  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        for (unsigned int l = 0; l < 3; ++l)
        {
          Real value = 0;
          for (unsigned int m = 0; m < 3; ++m)
            for (unsigned int n = 0; n < 3; ++n)
              value += A(m, i, j) * b(m, n) * A(n, k, l);
          EXPECT_EQ(D(i, j, k, l), value);
        }
}

TEST(RankThreeTensor, doubleContraction)
{
  RankThreeTensor A(std::vector<Real>(27, 1949));
  RankTwoTensor b(std::vector<Real>(9, 1954));
  VectorValue<Real> c = A.doubleContraction(b);

  for (unsigned int i = 0; i < 3; ++i)
  {
    Real value = 0;
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        value += A(i, j, k) * b(j, k);

    EXPECT_EQ(c(i), value);
  }
}

TEST(RankThreeTensor, ADConversion)
{
  RankThreeTensor reg;
  ADRankThreeTensor ad;

  ad = reg;
  reg = MetaPhysicL::raw_value(ad);

  GenericRankThreeTensor<false> generic_reg;
  GenericRankThreeTensor<true> generic_ad;

  generic_ad = generic_reg;
  generic_reg = MetaPhysicL::raw_value(generic_ad);
}
