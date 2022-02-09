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
#include "MooseADWrapper.h"
#include "InputParameters.h"

#include "libmesh/utility.h"
#include "libmesh/int_range.h"
#include "libmesh/vector_value.h"

#include "Eigen/Core"
#include <Eigen/Dense>
#include <unsupported/Eigen/NonLinearOptimization>

/**
 * Internal use namespace that contains template helpers to map each residual
 * type to its corresponding Jacobian type.
 */
namespace NestedSolveInternal
{
template <typename T>
struct CorrespondingJacobianTempl;
}

/**
 * NestedSolveTempl<is_ad> and its instantiations NestedSolve and ADNestedSolve
 * are utility classes that implement a non-linear solver. These classes can be
 * instantiated in any MOOSE object to perform local Newton-Raphson solves.
 */
template <bool is_ad>
class NestedSolveTempl
{
  /// AD/non-AD switched type shortcuts
  using NSReal = MooseADWrapper<Real, is_ad>;
  using NSRealVectorValue = MooseADWrapper<RealVectorValue, is_ad>;
  using NSRankTwoTensor = MooseADWrapper<RankTwoTensor, is_ad>;

public:
  static InputParameters validParams();

  NestedSolveTempl();
  NestedSolveTempl(const InputParameters & params);

  /// Eigen type shortcuts
  using DynamicVector = Eigen::Matrix<NSReal, Eigen::Dynamic, 1>;
  using DynamicMatrix = Eigen::Matrix<NSReal, Eigen::Dynamic, Eigen::Dynamic>;

  /// Residual and solution type
  template <int N = 0>
  using Value =
      typename std::conditional<N == 1,
                                Real,
                                typename std::conditional<N == 0,
                                                          NestedSolveTempl<is_ad>::DynamicVector,
                                                          Eigen::Matrix<NSReal, N, 1>>::type>::type;

  /// Jacobian matrix type
  template <int N = 0>
  using Jacobian =
      typename std::conditional<N == 1,
                                Real,
                                typename std::conditional<N == 0,
                                                          NestedSolveTempl<is_ad>::DynamicMatrix,
                                                          Eigen::Matrix<NSReal, N, N>>::type>::type;

  /// Solve the N*N nonlinear equation system using a built-in Netwon-Raphson loop
  template <typename V, typename T>
  void nonlinear(V & guess, T compute);

  /// @{ The separate residual/Jacobian functor versions use Eigen::HybridNonLinearSolver
  template <typename R, typename J>
  void nonlinear(DynamicVector & guess, R computeResidual, J computeJacobian);
  template <typename R, typename J>
  void nonlinear(NSReal & guess, R computeResidual, J computeJacobian);
  template <typename R, typename J>
  void nonlinear(NSRealVectorValue & guess, R computeResidual, J computeJacobian);
  ///@}

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

  /// possible solver states
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
  template <typename T>
  using CorrespondingJacobian = typename NestedSolveInternal::CorrespondingJacobianTempl<T>::type;
  ///@}

  /// Size a dynamic Jacobian matrix correctly
  void sizeItems(const NestedSolveTempl<is_ad>::DynamicVector & guess,
                 NestedSolveTempl<is_ad>::DynamicVector & residual,
                 NestedSolveTempl<is_ad>::DynamicMatrix & jacobian) const;

  /// Sizing is a no-op for compile time sized types (and scalars)
  template <typename V, typename T>
  void sizeItems(const V &, V &, T &) const
  {
  }

  ///@{ Solve A*x=b for x
  template <typename J, typename V>
  void linear(const J & A, V & x, const V & b) const;
  void linear(const NSReal & A, NSReal & x, const NSReal & b) const { x = b / A; }
  void linear(const NSRankTwoTensor & A, NSRealVectorValue & x, const NSRealVectorValue & b) const;
  ///@}

  ///@{ Compute squared norm of v (dropping derivatives as this is only for convergence checking)
  template <typename V>
  Real normSquare(const V & v) const;
  Real normSquare(const NSReal & v) const;
  Real normSquare(const NSRealVectorValue & v) const;
  ///@}
};

template <bool is_ad>
template <typename R, typename J>
void
NestedSolveTempl<is_ad>::nonlinear(NestedSolveTempl<is_ad>::DynamicVector & guess,
                                   R computeResidual,
                                   J computeJacobian)
{
  using V = NestedSolveTempl<is_ad>::DynamicVector;

  class EigenAdaptorFunctor
  {
  public:
    EigenAdaptorFunctor(R & residual, J & jacobian) : _residual(residual), _jacobian(jacobian) {}
    int operator()(V & guess, V & residual)
    {
      _residual(guess, residual);
      return 0;
    }
    int df(V & guess, NestedSolveTempl<is_ad>::CorrespondingJacobian<V> & jacobian)
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
  Eigen::HybridNonLinearSolver<decltype(functor), NSReal> solver(functor);
  auto status = solver.solve(guess);

  if (status == Eigen::HybridNonLinearSolverSpace::RelativeErrorTooSmall)
    _state = State::CONVERGED_REL;
  else
    _state = State::NOT_CONVERGED;
}

template <bool is_ad>
template <typename R, typename J>
void
NestedSolveTempl<is_ad>::nonlinear(NSReal & guess, R computeResidual, J computeJacobian)
{
  using V = NestedSolveTempl<is_ad>::DynamicVector;

  class EigenAdaptorFunctor
  {
  public:
    EigenAdaptorFunctor(R & residual, J & jacobian) : _residual(residual), _jacobian(jacobian) {}
    int operator()(V & guess, V & residual)
    {
      _residual(guess(0), residual(0, 0));
      return 0;
    }
    int df(V & guess, NestedSolveTempl<is_ad>::CorrespondingJacobian<V> & jacobian)
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
  Eigen::HybridNonLinearSolver<decltype(functor), NSReal> solver(functor);
  V guess_eigen(1);
  guess_eigen << guess;
  auto status = solver.solve(guess_eigen);
  guess = guess_eigen(0);

  if (status == Eigen::HybridNonLinearSolverSpace::RelativeErrorTooSmall)
    _state = State::CONVERGED_REL;
  else
    _state = State::NOT_CONVERGED;
}

template <bool is_ad>
template <typename R, typename J>
void
NestedSolveTempl<is_ad>::nonlinear(NSRealVectorValue & guess, R computeResidual, J computeJacobian)
{
  using V = NestedSolveTempl<is_ad>::DynamicVector;

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
    int df(V & guess, NestedSolveTempl<is_ad>::CorrespondingJacobian<V> & jacobian)
    {
      RealVectorValue guess_moose(guess(0), guess(1), guess(2));
      RankTwoTensor jacobian_moose;
      _jacobian(guess_moose, jacobian_moose);
      jacobian = Eigen::Map<NestedSolveTempl<is_ad>::DynamicMatrix>(&jacobian_moose(0, 0), 3, 3);
      return 0;
    }

  private:
    R & _residual;
    J & _jacobian;
  };

  // adaptor functor to utilize the Eigen non-linear solver
  auto functor = EigenAdaptorFunctor(computeResidual, computeJacobian);
  Eigen::HybridNonLinearSolver<decltype(functor), NSReal> solver(functor);
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

template <bool is_ad>
template <typename V, typename T>
void
NestedSolveTempl<is_ad>::nonlinear(V & guess, T compute)
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

template <bool is_ad>
template <typename J, typename V>
void
NestedSolveTempl<is_ad>::linear(const J & A, V & x, const V & b) const
{
  // we could make the linear solve method configurable here
  x = A.colPivHouseholderQr().solve(b);
}

template <bool is_ad>
template <typename V>
Real
NestedSolveTempl<is_ad>::normSquare(const V & v) const
{
  // we currently cannot get the raw_value of an AD Eigen Matrix, so we loop over components
  Real norm_square = 0.0;
  for (const auto i : make_range(v.size()))
    norm_square += Utility::pow<2>(MetaPhysicL::raw_value(v.data()[i]));
  return norm_square;
}

/**
 * Internal use namespace that contains template helpers to map each residual
 * type to its corresponding Jacobian type.
 */
namespace NestedSolveInternal
{

template <>
struct CorrespondingJacobianTempl<Real>
{
  using type = Real;
};

template <>
struct CorrespondingJacobianTempl<ADReal>
{
  using type = ADReal;
};

template <>
struct CorrespondingJacobianTempl<RealVectorValue>
{
  using type = RankTwoTensor;
};

template <>
struct CorrespondingJacobianTempl<ADRealVectorValue>
{
  using type = ADRankTwoTensor;
};

template <>
struct CorrespondingJacobianTempl<typename NestedSolveTempl<false>::DynamicVector>
{
  using type = NestedSolveTempl<false>::DynamicMatrix;
};

template <>
struct CorrespondingJacobianTempl<typename NestedSolveTempl<true>::DynamicVector>
{
  using type = NestedSolveTempl<true>::DynamicMatrix;
};

template <int N>
struct CorrespondingJacobianTempl<typename Eigen::Matrix<Real, N, 1>>
{
  using type = Eigen::Matrix<Real, N, N>;
};

template <int N>
struct CorrespondingJacobianTempl<typename Eigen::Matrix<ADReal, N, 1>>
{
  using type = Eigen::Matrix<ADReal, N, N>;
};

} // namespace NestedSolveInternal

typedef NestedSolveTempl<false> NestedSolve;
typedef NestedSolveTempl<true> ADNestedSolve;
