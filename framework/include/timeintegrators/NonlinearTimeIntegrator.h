//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
class SystemBase;
class NonlinearSystemBase;

namespace libMesh
{
template <typename T>
class NumericVector;
class NonlinearImplicitSystem;
} // namespace libMesh

/**
 * Interface class for routines and member variables for time integrators
 * relying on Newton's method.
 */
class NonlinearTimeIntegrator
{
public:
  NonlinearTimeIntegrator(FEProblemBase & problem, SystemBase & system);

  /**
   * Callback to the NonlinearTimeIntegrator called immediately after the
   * residuals are computed in NonlinearSystem::computeResidual().
   * The residual vector which is passed in to this function should
   * be filled in by the user with the _Re_time and _Re_non_time
   * vectors in a way that makes sense for the particular
   * TimeIntegration method.
   */
  virtual void postResidual(NumericVector<Number> & /*residual*/) {}

  /**
   * Returns the tag for the nodal multiplication factor for the residual calculation of the udot
   * term.
   *
   * By default, this tag will be associated with udot.
   */
  TagID uDotFactorTag() const { return _u_dot_factor_tag; }
  /**
   * Returns the tag for the nodal multiplication factor for the residual calculation of the udotdot
   * term.
   *
   * By default, this tag will be associated with udotdot.
   */
  TagID uDotDotFactorTag() const { return _u_dotdot_factor_tag; }

protected:
  /// Wrapper around vector addition for nonlinear time integrators. If we don't
  /// operate on a nonlinear system we don't need to add the vector.
  /// @param name The name of the vector
  /// @param project If the vector should be projected
  /// @param type PThe parallel distribution of the vetor
  NumericVector<Number> *
  addVectorForNonlinearTI(const std::string & name, const bool project, const ParallelType type);

  /// Pointer to the nonlinear system, can happen that we dont have any
  NonlinearSystemBase * _nl;

  /// Boolean to check if this integrator belongs to a nonlinear system
  const bool _integrates_nl;

  /// Nonlinear implicit system, if applicable; otherwise, nullptr
  NonlinearImplicitSystem * _nonlinear_implicit_system;

  /// residual vector for time contributions
  NumericVector<Number> * _Re_time;

  /// residual vector for non-time contributions
  NumericVector<Number> * _Re_non_time;

  /// The vector tag for the nodal multiplication factor for the residual calculation of the udot term
  const TagID _u_dot_factor_tag;

  /// The vector tag for the nodal multiplication factor for the residual calculation of the udotdot term
  const TagID _u_dotdot_factor_tag;
};
