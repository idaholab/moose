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
#include "RankTwoTensor.h"

#include "libmesh/vector_value.h"

TEST(NestedSolve, FixedSize)
{
  auto compute = [&](const NestedSolve::Value<2> & guess,
                     NestedSolve::Value<2> & residual,
                     NestedSolve::Jacobian<2> & jacobian)
  {
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

TEST(NestedSolve, DynamicSize)
{
  auto compute = [&](const NestedSolve::Value<> & guess,
                     NestedSolve::Value<> & residual,
                     NestedSolve::Jacobian<> & jacobian)
  {
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

TEST(NestedSolve, DynamicSizePowell)
{
  auto computeResidual = [&](const NestedSolve::Value<> & guess, NestedSolve::Value<> & residual)
  {
    residual(0) = guess(0) + guess(0) * guess(1) - 4;
    residual(1) = guess(0) + guess(1) - 3;
  };

  auto computeJacobian = [&](const NestedSolve::Value<> & guess, NestedSolve::Jacobian<> & jacobian)
  {
    jacobian(0, 0) = 1 + guess(1);
    jacobian(0, 1) = guess(0);
    jacobian(1, 0) = 1;
    jacobian(1, 1) = 1;
  };

  NestedSolve solver;
  NestedSolve::Value<> solution(2);
  solution << 1.98, 1.02;
  solver.nonlinear(solution, computeResidual, computeJacobian);

  EXPECT_NEAR(solution(0), 2, 1e-6);
  EXPECT_NEAR(solution(1), 1, 1e-6);
}

TEST(NestedSolve, RankTwoTensor)
{
  auto compute =
      [&](const RealVectorValue & guess, RealVectorValue & residual, RankTwoTensor & jacobian)
  {
    //  x + 2 * y - 2 * z + 15 = 0
    //  2 * x + y - 5 * z + 21 = 0
    //  x - 4 * y + z -18 = 0

    jacobian = RankTwoTensor(1, 2, -2, 2, 1, -5, 1, -4, 1).transpose();
    residual = jacobian * guess + RealVectorValue(15, 21, -18);
  };

  NestedSolve solver;
  RealVectorValue solution;
  solver.nonlinear(solution, compute);

  EXPECT_NEAR(solution(0), -1, 1e-6);
  EXPECT_NEAR(solution(1), -4, 1e-6);
  EXPECT_NEAR(solution(2), 3, 1e-6);
}

TEST(NestedSolve, RankTwoTensorPowell)
{
  auto computeResidual = [&](const RealVectorValue & guess, RealVectorValue & residual)
  {
    //  x + 2 * y - 2 * z + 15 = 0
    //  2 * x + y - 5 * z + 21 = 0
    //  x - 4 * y + z -18 = 0

    auto jacobian = RankTwoTensor(1, 2, -2, 2, 1, -5, 1, -4, 1).transpose();
    residual = jacobian * guess + RealVectorValue(15, 21, -18);
  };
  auto computeJacobian = [&](const RealVectorValue & /*guess*/, RankTwoTensor & jacobian)
  { jacobian = RankTwoTensor(1, 2, -2, 2, 1, -5, 1, -4, 1).transpose(); };

  NestedSolve solver;
  RealVectorValue solution;
  solver.nonlinear(solution, computeResidual, computeJacobian);

  EXPECT_NEAR(solution(0), -1, 1e-6);
  EXPECT_NEAR(solution(1), -4, 1e-6);
  EXPECT_NEAR(solution(2), 3, 1e-6);
}

TEST(NestedSolve, Scalar)
{
  auto compute = [&](const Real & guess, Real & residual, Real & jacobian)
  {
    residual = guess * guess * guess - 8;
    jacobian = 3 * guess * guess;
  };

  NestedSolve solver;
  Real solution = 1.0;
  solver.nonlinear(solution, compute);

  EXPECT_NEAR(solution, 2, 1e-6);
}

TEST(NestedSolve, ScalarPowell)
{
  auto computeResidual = [&](const Real & guess, Real & residual)
  { residual = guess * guess * guess - 8; };

  auto computeJacobian = [&](const Real & guess, Real & jacobian) { jacobian = 3 * guess * guess; };

  NestedSolve solver;
  Real solution = 1.0;
  solver.nonlinear(solution, computeResidual, computeJacobian);

  EXPECT_NEAR(solution, 2, 1e-6);
}

TEST(NestedSolve, PlacementNew)
{
  Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> em(3, 3);
  auto * rtp = new (em.data()) RankTwoTensor;
  auto & rt = *rtp;

  em << 1, 2, 3, 4, 5, 6, 7, 8, 9;
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      EXPECT_EQ(em(i, j), rt(i, j));
}

TEST(NestedSolve, NoConvergence)
{
  auto compute = [&](const Real & guess, Real & residual, Real & jacobian)
  {
    residual = guess * guess + 1.0;
    jacobian = 2.0 * guess;
  };

  NestedSolve solver;
  Real solution = 1.0;
  solver.nonlinear(solution, compute);

  EXPECT_TRUE(solver.getState() == NestedSolve::State::NOT_CONVERGED);
}
