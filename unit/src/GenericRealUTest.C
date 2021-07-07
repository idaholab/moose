//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GenericRealU.h"

TEST(GenericRealUTest, constructor)
{
  MooseUnits nondim("1");
  MooseUnits Pa("Pa");

  RealU v0;
  EXPECT_NEAR(v0.value(), 0, 1e-6);
  EXPECT_EQ(v0.unit(), nondim);

  RealU v1(1);
  EXPECT_NEAR(v1.value(), 1, 1e-6);
  EXPECT_EQ(v1.unit(), nondim);

  RealU v2(2, "Pa");
  EXPECT_NEAR(v2.value(), 2, 1e-6);
  EXPECT_EQ(v2.unit(), Pa);

  RealU v3(3, Pa);
  EXPECT_NEAR(v3.value(), 3, 1e-6);
  EXPECT_EQ(v3.unit(), Pa);
}

TEST(GenericRealUTest, assignment)
{
  MooseUnits K("K");
  MooseUnits degF("degF");
  RealU to(K);
  ADRealU ad_to(K);
  ADReal v;

  to = 100;
  EXPECT_NEAR(to.value(), 100, 1e-6);

  to = RealU(200, degF);
  EXPECT_NEAR(to.value(), K.convert(200, degF), 1e-6);

  v = 100;
  Moose::derivInsert(v.derivatives(), 0, 1);
  ad_to = v;
  EXPECT_NEAR(raw_value(ad_to.value()), 100, 1e-6);
  EXPECT_NEAR(ad_to.value().derivatives()[0], 1, 1e-6);

  v = 200;
  Moose::derivInsert(v.derivatives(), 0, 9);
  ad_to = ADRealU(v, degF);
  EXPECT_NEAR(raw_value(ad_to.value()), K.convert(200, degF), 1e-6);
  EXPECT_NEAR(ad_to.value().derivatives()[0], 5, 1e-6);
}

TEST(GenericRealUTest, addition)
{
  MooseUnits degC("degC");
  MooseUnits degF("degF");
  MooseUnits K("K");
  RealU a(100, degC);
  RealU b(212, degF);
  RealU c(K);

  c = a + 100;
  EXPECT_NEAR(c.value(), K.convert(200, degC), 1e-6);

  c = a + b;
  EXPECT_NEAR(c.value(), K.convert(200, degC), 1e-6);

  c = a;
  c += 100;
  EXPECT_NEAR(c.value(), K.convert(a.value(), degC) + 100, 1e-6);

  c = a;
  c += b;
  EXPECT_NEAR(c.value(), K.convert(a.value(), degC) + K.convert(b.value(), degF), 1e-6);
}

TEST(GenericRealUTest, subtraction)
{
  MooseUnits degC("degC");
  MooseUnits degF("degF");
  MooseUnits K("K");
  RealU a(100, degC);
  RealU b(212, degF); // 100 degC
  RealU c(K);

  c = a - 100;
  EXPECT_NEAR(c.value(), K.convert(0, degC), 1e-6);

  c = a - b;
  EXPECT_NEAR(c.value(), K.convert(0, degC), 1e-6);

  c = a;
  c -= 100;
  EXPECT_NEAR(c.value(), K.convert(a.value(), degC) - 100, 1e-6);

  c = a;
  c -= b;
  EXPECT_NEAR(c.value(), K.convert(a.value(), degC) - K.convert(b.value(), degF), 1e-6);
}

TEST(GenericRealUTest, multiplication)
{
  RealU a(5, "m");
  RealU b(3000, "mm"); // 3 m
  RealU c("m^2");

  c = a * 3;
  EXPECT_NEAR(c.value(), 15, 1e-6);

  c = a * b;
  EXPECT_NEAR(c.value(), 15, 1e-6);
}

TEST(GenericRealUTest, division)
{
  RealU a(3, "m");
  RealU b(5000, "mm"); // 5 m
  RealU c("1");

  c = a / 5;
  EXPECT_NEAR(c.value(), 0.6, 1e-6);

  c = a / b;
  EXPECT_NEAR(c.value(), 0.6, 1e-6);
}

TEST(GenericRealUTest, power)
{
  RealU a(3, "m");
  RealU b("m^3");

  b = std::pow(a, 3);
  EXPECT_NEAR(b.value(), 27, 1e-6);
}

TEST(GenericRealUTest, abs)
{
  RealU a(-3000, "mm");
  RealU b("m");
  b = std::abs(a);
  EXPECT_NEAR(b.value(), 3, 1e-6);
}

TEST(GenericRealUTest, comparison)
{
  RealU a(3, "kPa");
  RealU b(5, "kPa");
  RealU c(3000, "Pa");

  EXPECT_EQ(a > b, false);
  EXPECT_EQ(a >= b, false);
  EXPECT_EQ(a < b, true);
  EXPECT_EQ(a <= b, true);
  EXPECT_EQ(a == b, false);

  EXPECT_EQ(a > c, false);
  EXPECT_EQ(a >= c, true);
  EXPECT_EQ(a < c, false);
  EXPECT_EQ(a <= c, true);
  EXPECT_EQ(a == c, true);

  EXPECT_EQ(b > c, true);
  EXPECT_EQ(b >= c, true);
  EXPECT_EQ(b < c, false);
  EXPECT_EQ(b <= c, false);
  EXPECT_EQ(b == c, false);
}
