//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseUtils.h"
#include <cmath>

TEST(FuzzyComparisons, fuzzyFunctions)
{
  libMesh::Real tol = libMesh::TOLERANCE * libMesh::TOLERANCE;
  libMesh::Real a = 1.0;
  libMesh::Real aplus = a + 0.5 * tol;
  libMesh::Real aminus = a - 0.5 * tol;
  libMesh::Real aplus2 = a + 1.5 * tol;
  libMesh::Real aminus2 = a - 1.5 * tol;

  libMesh::Real b = 1000.0;
  libMesh::Real rtol = tol * 2.0 * b;
  libMesh::Real bplus = b + 0.5 * rtol;
  libMesh::Real bminus = b - 0.5 * rtol;
  libMesh::Real bplus2 = b + 1.5 * rtol;
  libMesh::Real bminus2 = b - 1.5 * rtol;
  libMesh::Real zero = 0.0;

  ASSERT_TRUE(MooseUtils::absoluteFuzzyEqual(zero, zero));
  ASSERT_TRUE(MooseUtils::absoluteFuzzyEqual(tol, tol));
  ASSERT_TRUE(MooseUtils::absoluteFuzzyEqual(a, aplus));
  ASSERT_TRUE(!MooseUtils::absoluteFuzzyEqual(a, aplus2));

  ASSERT_TRUE(!MooseUtils::absoluteFuzzyGreaterThan(aplus, a));
  ASSERT_TRUE(MooseUtils::absoluteFuzzyGreaterThan(aplus2, a));

  ASSERT_TRUE(MooseUtils::absoluteFuzzyGreaterEqual(aplus, a));
  ASSERT_TRUE(MooseUtils::absoluteFuzzyGreaterEqual(aminus, a));
  ASSERT_TRUE(MooseUtils::absoluteFuzzyGreaterEqual(aplus2, a));
  ASSERT_TRUE(!MooseUtils::absoluteFuzzyGreaterEqual(aminus2, a));

  ASSERT_TRUE(!MooseUtils::absoluteFuzzyLessThan(aminus, a));
  ASSERT_TRUE(MooseUtils::absoluteFuzzyLessThan(aminus2, a));

  ASSERT_TRUE(MooseUtils::absoluteFuzzyLessEqual(aminus, a));
  ASSERT_TRUE(MooseUtils::absoluteFuzzyLessEqual(aplus, a));
  ASSERT_TRUE(MooseUtils::absoluteFuzzyLessEqual(aminus2, a));
  ASSERT_TRUE(!MooseUtils::absoluteFuzzyLessEqual(aplus2, a));

  ASSERT_TRUE(MooseUtils::relativeFuzzyEqual(zero, zero));
  ASSERT_TRUE(MooseUtils::relativeFuzzyEqual(tol, tol));
  ASSERT_TRUE(!MooseUtils::relativeFuzzyEqual(zero, tol));
  ASSERT_TRUE(MooseUtils::relativeFuzzyEqual(b, bplus));
  ASSERT_TRUE(!MooseUtils::relativeFuzzyEqual(b, bplus2));

  ASSERT_TRUE(!MooseUtils::relativeFuzzyGreaterThan(bplus, b));
  ASSERT_TRUE(MooseUtils::relativeFuzzyGreaterThan(bplus2, b));

  ASSERT_TRUE(MooseUtils::relativeFuzzyGreaterEqual(bplus, b));
  ASSERT_TRUE(MooseUtils::relativeFuzzyGreaterEqual(bminus, b));
  ASSERT_TRUE(MooseUtils::relativeFuzzyGreaterEqual(bplus2, b));
  ASSERT_TRUE(!MooseUtils::relativeFuzzyGreaterEqual(bminus2, b));

  ASSERT_TRUE(!MooseUtils::relativeFuzzyLessThan(bminus, b));
  ASSERT_TRUE(MooseUtils::relativeFuzzyLessThan(bminus2, b));

  ASSERT_TRUE(MooseUtils::relativeFuzzyLessEqual(bminus, b));
  ASSERT_TRUE(MooseUtils::relativeFuzzyLessEqual(bplus, b));
  ASSERT_TRUE(MooseUtils::relativeFuzzyLessEqual(bminus2, b));
  ASSERT_TRUE(!MooseUtils::relativeFuzzyLessEqual(bplus2, b));
}
