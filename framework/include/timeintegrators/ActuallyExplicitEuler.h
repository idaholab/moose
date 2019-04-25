//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeIntegrator.h"
#include "MeshChangedInterface.h"

#include "libmesh/linear_solver.h"

// Forward declarations
class ActuallyExplicitEuler;
class LumpedPreconditioner;

template <>
InputParameters validParams<ActuallyExplicitEuler>();

/**
 * Implements a truly explicit (no nonlinear solve) first-order, forward Euler
 * time integration scheme.
 */
class ActuallyExplicitEuler : public TimeIntegrator, public MeshChangedInterface
{
public:
  ActuallyExplicitEuler(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void init() override;
  virtual void preSolve() override;
  virtual int order() override { return 1; }
  virtual void computeTimeDerivatives() override;
  virtual void computeADTimeDerivatives(DualReal & ad_u_dot, const dof_id_type & dof) override;
  virtual void solve() override;
  virtual void postResidual(NumericVector<Number> & residual) override;

  virtual void meshChanged() override;

protected:
  enum SolveType
  {
    CONSISTENT,
    LUMPED,
    LUMP_PRECONDITIONED
  };

  /**
   * Check for the linear solver convergence
   */
  bool checkLinearConvergence();

  /**
   * Helper function that actually does the math for computing the time derivative
   */
  template <typename T, typename T2>
  void computeTimeDerivativeHelper(T & u_dot, const T2 & u_old);

  MooseEnum _solve_type;

  /// Residual used for the RHS
  NumericVector<Real> & _explicit_residual;

  /// Solution vector for the linear solve
  NumericVector<Real> & _explicit_euler_update;

  /// Diagonal of the lumped mass matrix (and its inversion)
  NumericVector<Real> & _mass_matrix_diag;

  /// Just a vector of 1's to help with creating the lumped mass matrix
  NumericVector<Real> * _ones;

  /// For computing the mass matrix
  TagID _Ke_time_tag;

  /// For solving with the consistent matrix
  std::unique_ptr<LinearSolver<Number>> _linear_solver;

  /// For solving with lumped preconditioning
  std::unique_ptr<LumpedPreconditioner> _preconditioner;

  /// Save off current time to reset it back and forth
  Real _current_time;
};

template <typename T, typename T2>
void
ActuallyExplicitEuler::computeTimeDerivativeHelper(T & u_dot, const T2 & u_old)
{
  u_dot -= u_old;
  u_dot *= 1. / _dt;
}

