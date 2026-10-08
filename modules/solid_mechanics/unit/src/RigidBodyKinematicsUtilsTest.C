//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ADReal.h"
#include "RigidBodyKinematicsUtils.h"

using namespace RigidBodyKinematicsUtils;

TEST(RigidBodyKinematicsUtilsTest, rigidDisplacement)
{
  // Compare with the rotation tensor of an anticlockwise rotation by 1.3 about (2, -1, 2) / 3
  const RealVectorValue n = RealVectorValue(2, -1, 2) / 3.0;
  const Real phi = 1.3;
  const RealTensorValue K(0, -n(2), n(1), n(2), 0, -n(0), -n(1), n(0), 0);
  const RealTensorValue R =
      RealTensorValue(1, 0, 0, 0, 1, 0, 0, 0, 1) + std::sin(phi) * K + (1 - std::cos(phi)) * K * K;

  const RealVectorValue translation(0.1, -0.2, 0.3);
  const Point X(1.5, -0.5, 2.0);
  const Point X_ref(0.2, 0.3, -0.1);
  const RealVectorValue theta = phi * n;
  const RealVectorValue expected = translation + R * (X - X_ref) - (X - X_ref);
  EXPECT_NEAR((rigidDisplacement(translation, theta, X, X_ref) - expected).norm(), 0.0, 1e-14);
}

TEST(RigidBodyKinematicsUtilsTest, seriesBranchContinuity)
{
  // The coefficients switch from the Taylor series to the closed form at s = 1e-2
  const auto [a_below, b_below] = rodriguesCoefficients<Real>(1e-2 * (1 - 1e-12));
  const auto [a_above, b_above] = rodriguesCoefficients<Real>(1e-2 * (1 + 1e-12));
  EXPECT_NEAR(a_below, a_above, 1e-14);
  EXPECT_NEAR(b_below, b_above, 1e-14);
  const auto [a0, b0] = rodriguesCoefficients<Real>(0);
  EXPECT_EQ(a0, 1.0);
  EXPECT_EQ(b0, 0.5);
}

TEST(RigidBodyKinematicsUtilsTest, derivatives)
{
  // At theta = 0 the derivative of U with respect to theta is exactly -[X - X_ref]_x
  {
    ADRealVectorValue theta;
    for (const auto i : make_range(3))
      Moose::derivInsert(theta(i).derivatives(), i, 1.0);
    const auto U = rigidDisplacement(ADRealVectorValue(), theta, Point(1.0, 2.0, 3.0), Point());
    const RealTensorValue expected(0, 3, -2, -3, 0, 1, 2, -1, 0);
    for (const auto i : make_range(3))
      for (const auto j : make_range(3))
        EXPECT_NEAR(U(i).derivatives()[j], expected(i, j), 1e-14);
  }

  // Elsewhere the AD derivatives agree with central differences, on both sides of the series
  // threshold
  const Point X(0.7, -1.3, 0.4);
  const Point X_ref(0.1, 0.2, -0.3);
  for (const auto & theta0 : {RealVectorValue(0.3, -1.1, 0.7), RealVectorValue(0.02, 0.05, -0.07)})
  {
    ADRealVectorValue theta = theta0;
    for (const auto i : make_range(3))
      Moose::derivInsert(theta(i).derivatives(), i, 1.0);
    const auto U = rigidDisplacement(ADRealVectorValue(), theta, X, X_ref);

    // Step balancing truncation and roundoff errors of the central difference
    const Real h = 1e-6;
    for (const auto j : make_range(3))
    {
      RealVectorValue theta_p = theta0, theta_m = theta0;
      theta_p(j) += h;
      theta_m(j) -= h;
      const RealVectorValue dU = (rigidDisplacement(RealVectorValue(), theta_p, X, X_ref) -
                                  rigidDisplacement(RealVectorValue(), theta_m, X, X_ref)) /
                                 (2 * h);
      for (const auto i : make_range(3))
        EXPECT_NEAR(U(i).derivatives()[j], dU(i), 1e-8);
    }
  }
}
