//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose includes
#include "MooseTypes.h"

// Libmesh includes
#include "libmesh/enum_parallel_type.h"

// Forward declarations
class SystemBase;
class FEProblemBase;
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
class NonlinearTimeIntegratorInterface
{
public:
  NonlinearTimeIntegratorInterface(FEProblemBase & problem, SystemBase & system);

  /**
   * Callback to the NonLinearTimeIntegratorInterface called immediately after the
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
  /// @param type The parallel distribution of the vetor
  NumericVector<Number> *
  addVector(const std::string & name, const bool project, const libMesh::ParallelType type);

  /// Pointer to the nonlinear system, can happen that we dont have any
  NonlinearSystemBase * _nl;

  /// libMesh nonlinear implicit system, if applicable; otherwise, nullptr
  libMesh::NonlinearImplicitSystem * _nonlinear_implicit_system;

  /// residual vector for time contributions
  NumericVector<Number> * _Re_time;

  /// residual vector for non-time contributions
  NumericVector<Number> * _Re_non_time;

  /// The vector tag for the nodal multiplication factor for the residual calculation of the udot term
  const TagID _u_dot_factor_tag;

  /// The vector tag for the nodal multiplication factor for the residual calculation of the udotdot term
  const TagID _u_dotdot_factor_tag;
};
