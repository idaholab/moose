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
namespace NestedSolve
{
template <int N = 0>
using Value =
    typename std::conditional<N == 1,
                              Real,
                              typename std::conditional<N == 0,
                                                        Eigen::Matrix<Real, Eigen::Dynamic, 1>,
                                                        Eigen::Matrix<Real, N, 1>>::type>::type;

template <int N = 0>
using Jacobian = typename std::conditional<
    N == 1,
    Real,
    typename std::conditional<N == 0,
                              Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic>,
                              Eigen::Matrix<Real, N, N>>::type>::type;

template <typename T>
struct CorrespondingJacobianTempl;

template <>
struct CorrespondingJacobianTempl<Real>
{
  using type = Real;
};

template <>
struct CorrespondingJacobianTempl<Eigen::Matrix<Real, Eigen::Dynamic, 1>>
{
  using type = Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic>;
};

template <int N>
struct CorrespondingJacobianTempl<Eigen::Matrix<Real, N, 1>>
{
  using type = Eigen::Matrix<Real, N, N>;
};

template <typename T>
using CorrespondingJacobian = typename CorrespondingJacobianTempl<T>::type;

/**
 * Size a dynamic Jacobian matrix correctly
 */
template <typename V, typename T>
void
sizeItems(const V &, V &, T &)
{
}

void
sizeItems(const Eigen::Matrix<Real, Eigen::Dynamic, 1> & guess,
          Eigen::Matrix<Real, Eigen::Dynamic, 1> & residual,
          Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic> & jacobian)
{
  const auto N = guess.size();
  residual.resize(N, 1);
  jacobian.resize(N, N);
}

/**
 *  Solve A*x=b for x
 */
template <typename J, typename V>
void
linear(const J & A, V & x, const V & b)
{
  x = A.colPivHouseholderQr().solve(b);
}

void
linear(Real A, Real & x, Real b)
{
  x = b / A;
}

/**
 * Compute squared norm of v
 */
template <typename V>
Real
normSquare(const V & v)
{
  return v.squaredNorm();
}

Real
normSquare(Real v)
{
  return v * v;
}

/**
 * Solve the N*N nonlinear equation system
 */
template <typename V, typename T>
void
nonlinear(V & guess, T compute)
{
  V delta;
  V residual;
  CorrespondingJacobian<V> jacobian;
  sizeItems(guess, residual, jacobian);

  std::size_t n_iterations = 0;
  while (n_iterations < 15)
  {
    compute(guess, residual, jacobian);
    if (normSquare(residual) < 1e-30)
      return;

    linear(jacobian, delta, residual);
    guess -= delta;
    n_iterations++;
  }
}

}
}
