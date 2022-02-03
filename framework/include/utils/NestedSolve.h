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
#include "RankTwoTensor.h"
#include "InputParameters.h"

#include "libmesh/vector_value.h"

#include "Eigen/Core"
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>

class NestedSolve
{
public:
  static InputParameters validParams();

  NestedSolve();
  NestedSolve(const InputParameters & params);

  typedef Eigen::Matrix<Real, Eigen::Dynamic, 1> DynamicVector;
  typedef Eigen::Matrix<Real, Eigen::Dynamic, Eigen::Dynamic> DynamicMatrix;

  /// Resiudual and Solution type
  template <int N = 0>
  using Value =
      typename std::conditional<N == 1,
                                Real,
                                typename std::conditional<N == 0,
                                                          NestedSolve::DynamicVector,
                                                          Eigen::Matrix<Real, N, 1>>::type>::type;

  /// Jacobian matrix type
  template <int N = 0>
  using Jacobian =
      typename std::conditional<N == 1,
                                Real,
                                typename std::conditional<N == 0,
                                                          NestedSolve::DynamicMatrix,
                                                          Eigen::Matrix<Real, N, N>>::type>::type;

  /**
   * Solve the N*N nonlinear equation system
   */
  template <typename V, typename T>
  void nonlinear(V & guess, T compute);

  template <typename R, typename J>
  void nonlinear(DynamicVector & guess, R computeResidual, J computeJacobian);
  template <typename R, typename J>
  void nonlinear(Real & guess, R computeResidual, J computeJacobian);
  template <typename R, typename J>
  void nonlinear(RealVectorValue & guess, R computeResidual, J computeJacobian);

  ///@{ default values
  static Real relativeToleranceDefault() { return 1e-8; }
  static Real absoluteToleranceDefault() { return 1e-13; }
  static unsigned int minIterationsDefault() { return 10; }
  static unsigned int maxIterationsDefault() { return 1000; }
  ///@}

  void setRelativeTolerance(Real rel) { _relative_tolerance_square = rel * rel; }
  void setAbsoluteTolerance(Real abs) { _absolute_tolerance_square = abs * abs; }

  Real _relative_tolerance_square;
  Real _absolute_tolerance_square;
  unsigned int _min_iterations;
  unsigned int _max_iterations;

  enum class State
  {
    NONE,
    CONVERGED_ABS,
    CONVERGED_REL,
    EXACT_GUESS,
    NOT_CONVERGED
  };

  /// Get the solver state
  const State & getState() const { return _state; }
  /// Get the number of iterations from the last solve
  const std::size_t & getIterations() { return _n_iterations; };

protected:
  /// current solver state
  State _state;

  /// number of nested iterations
  std::size_t _n_iterations;

  ///@{ Deduce the Jacobian type from the solution type
  template <typename T>
  struct CorrespondingJacobianTempl;

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
  void sizeItems(const NestedSolve::DynamicVector & guess,
                 NestedSolve::DynamicVector & residual,
                 NestedSolve::DynamicMatrix & jacobian) const;

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
  void linear(RankTwoTensor A, RealVectorValue & x, RealVectorValue b) const;

  /**
   * Compute squared norm of v
   */
  template <typename V>
  Real normSquare(const V & v) const
  {
    return v.squaredNorm();
  }

  Real normSquare(Real v) const { return v * v; }
  Real normSquare(RealVectorValue v) const;
};

template <>
struct NestedSolve::CorrespondingJacobianTempl<Real>
{
  using type = Real;
};

template <>
struct NestedSolve::CorrespondingJacobianTempl<RealVectorValue>
{
  using type = RankTwoTensor;
};

template <>
struct NestedSolve::CorrespondingJacobianTempl<NestedSolve::DynamicVector>
{
  using type = NestedSolve::DynamicMatrix;
};

template <typename R, typename J>
void
NestedSolve::nonlinear(NestedSolve::DynamicVector & guess, R computeResidual, J computeJacobian)
{
  typedef NestedSolve::DynamicVector V;

  class EigenAdaptorFunctor
  {
  public:
    EigenAdaptorFunctor(R & residual, J & jacobian) : _residual(residual), _jacobian(jacobian) {}
    int operator()(V & guess, V & residual)
    {
      _residual(guess, residual);
      return 0;
    }
    int df(V & guess, NestedSolve::CorrespondingJacobian<V> & jacobian)
    {
      _jacobian(guess, jacobian);
      return 0;
    }

  private:
    R & _residual;
    J & _jacobian;
  };

  // adaptor functor to utilize the Eigen non-linear solver
  auto functor = EigenAdaptorFunctor(computeResidual, computeJacobian);
  Eigen::HybridNonLinearSolver<decltype(functor)> solver(functor);
  auto status = solver.solve(guess);

  if (status == Eigen::HybridNonLinearSolverSpace::RelativeErrorTooSmall)
    _state = State::CONVERGED_REL;
  else
    _state = State::NOT_CONVERGED;
}

template <typename R, typename J>
void
NestedSolve::nonlinear(Real & guess, R computeResidual, J computeJacobian)
{
  typedef NestedSolve::DynamicVector V;

  class EigenAdaptorFunctor
  {
  public:
    EigenAdaptorFunctor(R & residual, J & jacobian) : _residual(residual), _jacobian(jacobian) {}
    int operator()(V & guess, V & residual)
    {
      _residual(guess(0), residual(0, 0));
      return 0;
    }
    int df(V & guess, NestedSolve::CorrespondingJacobian<V> & jacobian)
    {
      _jacobian(guess(0), jacobian(0, 0));
      return 0;
    }

  private:
    R & _residual;
    J & _jacobian;
  };

  // adaptor functor to utilize the Eigen non-linear solver
  auto functor = EigenAdaptorFunctor(computeResidual, computeJacobian);
  Eigen::HybridNonLinearSolver<decltype(functor)> solver(functor);
  V guess_eigen(1);
  guess_eigen << guess;
  auto status = solver.solve(guess_eigen);
  guess = guess_eigen(0);

  if (status == Eigen::HybridNonLinearSolverSpace::RelativeErrorTooSmall)
    _state = State::CONVERGED_REL;
  else
    _state = State::NOT_CONVERGED;
}

template <typename R, typename J>
void
NestedSolve::nonlinear(RealVectorValue & guess, R computeResidual, J computeJacobian)
{
  typedef NestedSolve::DynamicVector V;

  class EigenAdaptorFunctor
  {
  public:
    EigenAdaptorFunctor(R & residual, J & jacobian) : _residual(residual), _jacobian(jacobian) {}
    int operator()(V & guess, V & residual)
    {
      RealVectorValue guess_moose(guess(0), guess(1), guess(2));
      RealVectorValue residual_moose;
      _residual(guess_moose, residual_moose);
      residual(0) = residual_moose(0);
      residual(1) = residual_moose(1);
      residual(2) = residual_moose(2);
      return 0;
    }
    int df(V & guess, NestedSolve::CorrespondingJacobian<V> & jacobian)
    {
      RealVectorValue guess_moose(guess(0), guess(1), guess(2));
      RankTwoTensor jacobian_moose;
      _jacobian(guess_moose, jacobian_moose);
      jacobian = Eigen::Map<NestedSolve::DynamicMatrix>(&jacobian_moose(0, 0), 3, 3);
      return 0;
    }

  private:
    R & _residual;
    J & _jacobian;
  };

  // adaptor functor to utilize the Eigen non-linear solver
  auto functor = EigenAdaptorFunctor(computeResidual, computeJacobian);
  Eigen::HybridNonLinearSolver<decltype(functor)> solver(functor);
  V guess_eigen(3);
  guess_eigen << guess(0), guess(1), guess(2);
  auto status = solver.solve(guess_eigen);
  guess(0) = guess_eigen(0);
  guess(1) = guess_eigen(1);
  guess(2) = guess_eigen(2);

  if (status == Eigen::HybridNonLinearSolverSpace::RelativeErrorTooSmall)
    _state = State::CONVERGED_REL;
  else
    _state = State::NOT_CONVERGED;
}

template <typename V, typename T>
void
NestedSolve::nonlinear(V & guess, T compute)
{
  V delta;
  V residual;
  CorrespondingJacobian<V> jacobian;
  sizeItems(guess, residual, jacobian);

  _n_iterations = 0;
  compute(guess, residual, jacobian);

  // compute first residual norm for relative convergence checks
  auto r0_square = normSquare(residual);
  if (r0_square == 0)
  {
    _state = State::EXACT_GUESS;
    return;
  }
  auto r_square = r0_square;

  // lambda to check for convergence and set the state accordingly
  auto is_converged = [&]()
  {
    if (r_square < _absolute_tolerance_square)
    {
      _state = State::CONVERGED_ABS;
      return true;
    }
    if (r_square / r0_square < _relative_tolerance_square)
    {
      _state = State::CONVERGED_REL;
      return true;
    }
    return false;
  };

  // perform non-linear iterations
  while (_n_iterations < _max_iterations)
  {
    // check convergence
    if (_n_iterations >= _min_iterations && is_converged())
      return;

    // solve and apply next increment
    linear(jacobian, delta, residual);
    guess -= delta;
    _n_iterations++;

    // compute residual and jacobian for the next iteration
    compute(guess, residual, jacobian);
    r_square = normSquare(residual);
  }

  // if we exceed the max iterations, we could still be converged
  if (!is_converged())
    _state = State::NOT_CONVERGED;
}
