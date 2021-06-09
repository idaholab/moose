//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "Eigen/Core"
#include <Eigen/Dense>

namespace Moose
{
namespace FixedPoint
{
template <int N>
using Jacobian = typename std::conditional<N == 1, Real, Eigen::Matrix<Real, N, N>>::type;

template <int N>
using Value = typename std::conditional<N == 1, Real, Eigen::Matrix<Real, N, 1>>::type;

/**
 *  Solve A*x=b for x
 */
template <int N>
void
linearSolve(const Jacobian<N> & A, Value<N> & x, const Value<N> & b)
{
  x = A.colPivHouseholderQr().solve(b);
}

template <>
void
linearSolve<1>(const Jacobian<1> & A, Value<1> & x, const Value<1> & b)
{
  x = b / A;
}

/**
 * Compute squared norm of v
 */
template <int N>
Real
normSquare(const Value<N> & v)
{
  return v.squaredNorm();
}

template <>
Real
normSquare<1>(const Value<1> & v)
{
  return v * v;
}

/**
 * Solve the N*N nonlinear equation system
 */
template <int N, typename T>
void
nonlinearSolve(Value<N> & guess, T compute)
{
  Value<N> delta;
  Value<N> residual;
  Jacobian<N> jacobian;

  std::size_t n_iterations = 0;
  while (n_iterations < 10)
  {
    compute(guess, residual, jacobian);
    if (normSquare<N>(residual) < 1e-30)
      return;

    linearSolve<N>(jacobian, delta, residual);
    guess -= delta;
    n_iterations++;
  }
}

}
}
