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
#include "InputParameters.h"

#include "Eigen/Core"
#include <Eigen/Dense>

class NestedSolve
{
public:
  static InputParameters validParams();

  NestedSolve();
  NestedSolve(const InputParameters & params);

  /// Resiudual and Solution type
  template <int N = 0>
  using Value =
      typename std::conditional<N == 1,
                                Real,
                                typename std::conditional<N == 0,
                                                          Eigen::Matrix<Real, Eigen::Dynamic, 1>,
                                                          Eigen::Matrix<Real, N, 1>>::type>::type;

  /// Jacobian matrix type
  template <int N = 0>
  using Jacobian = typename std::conditional<
      N == 1,
      Real,
      typename std::conditional<N == 0,
                                Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic>,
                                Eigen::Matrix<Real, N, N>>::type>::type;

  /**
   * Solve the N*N nonlinear equation system
   */
  template <typename V, typename T>
  void nonlinear(V & guess, T compute) const;

  ///@{ default values
  static Real relativeToleranceDefault() { return 1e-8; }
  static Real absoluteToleranceDefault() { return 1e-8; }
  static unsigned int maxIterationsDefault() { return 20; }
  ///@}

  Real _relative_tolerance_square;
  Real _absolute_tolerance_square;
  unsigned int _max_iterations;

protected:
  ///@{ Deduce the Jacobian type from the solution type
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
  ///@}

  /**
   * Size a dynamic Jacobian matrix correctly
   */
  void sizeItems(const Eigen::Matrix<Real, Eigen::Dynamic, 1> & guess,
                 Eigen::Matrix<Real, Eigen::Dynamic, 1> & residual,
                 Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic> & jacobian) const
  {
    const auto N = guess.size();
    residual.resize(N, 1);
    jacobian.resize(N, N);
  }

  /**
   * Sizing is a no-op for compile time sized types (and scalars)
   */
  template <typename V, typename T>
  void sizeItems(const V &, V &, T &) const
  {
  }

  /**
   *  Solve A*x=b for x
   */
  template <typename J, typename V>
  void linear(const J & A, V & x, const V & b) const
  {
    // we could make the linear solve method configurable here
    x = A.colPivHouseholderQr().solve(b);
  }

  void linear(Real A, Real & x, Real b) const { x = b / A; }

  /**
   * Compute squared norm of v
   */
  template <typename V>
  Real normSquare(const V & v) const
  {
    return v.squaredNorm();
  }

  Real normSquare(Real v) const { return v * v; }
};

template <typename V, typename T>
void
NestedSolve::nonlinear(V & guess, T compute) const
{
  V delta;
  V residual;
  CorrespondingJacobian<V> jacobian;
  sizeItems(guess, residual, jacobian);

  std::size_t n_iterations = 0;
  while (n_iterations < _max_iterations)
  {
    compute(guess, residual, jacobian);
    if (normSquare(residual) < 1e-30)
      return;

    linear(jacobian, delta, residual);
    guess -= delta;
    n_iterations++;
  }
}
