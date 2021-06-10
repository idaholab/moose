//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "NestedSolve.h"

TEST(FixedSize, test)
{
  auto compute = [&](const NestedSolve::Value<2> & guess,
                     NestedSolve::Value<2> & residual,
                     NestedSolve::Jacobian<2> & jacobian) {
    residual(0) = guess(0) + guess(0) * guess(1) - 4;
    residual(1) = guess(0) + guess(1) - 3;

    jacobian(0, 0) = 1 + guess(1);
    jacobian(0, 1) = guess(0);
    jacobian(1, 0) = 1;
    jacobian(1, 1) = 1;
  };

  NestedSolve solver;
  NestedSolve::Value<2> solution{1.98, 1.02};
  solver.setRelativeTolerance(1e-10);
  solver.nonlinear(solution, compute);

  EXPECT_NEAR(solution(0), 2, 1e-6);
  EXPECT_NEAR(solution(1), 1, 1e-6);
}

TEST(DynamicSize, test)
{
  auto compute = [&](const NestedSolve::Value<> & guess,
                     NestedSolve::Value<> & residual,
                     NestedSolve::Jacobian<> & jacobian) {
    residual(0) = guess(0) + guess(0) * guess(1) - 4;
    residual(1) = guess(0) + guess(1) - 3;

    jacobian(0, 0) = 1 + guess(1);
    jacobian(0, 1) = guess(0);
    jacobian(1, 0) = 1;
    jacobian(1, 1) = 1;
  };

  NestedSolve solver;
  NestedSolve::Value<> solution(2);
  solution << 1.98, 1.02;
  solver.setRelativeTolerance(1e-10);
  solver.nonlinear(solution, compute);

  EXPECT_NEAR(solution(0), 2, 1e-6);
  EXPECT_NEAR(solution(1), 1, 1e-6);
}

TEST(Scalar, test)
{
  auto compute = [&](const Real & guess, Real & residual, Real & jacobian) {
    residual = guess * guess * guess - 8;
    jacobian = 3 * guess * guess;
  };

  NestedSolve solver;
  Real solution = 1.0;
  solver.nonlinear(solution, compute);

  EXPECT_NEAR(solution, 2, 1e-6);
}
