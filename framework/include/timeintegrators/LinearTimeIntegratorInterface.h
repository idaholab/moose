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

// Forward declarations
class SystemBase;
class LinearSystem;

namespace libMesh
{
template <typename T>
class NumericVector;
class LinearImplicitSystem;
} // namespace libMesh

/**
 * Interface class for routines and member variables for time integrators
 * relying on linear system assembly method.
 */
class LinearTimeIntegratorInterface
{
public:
  LinearTimeIntegratorInterface(SystemBase & system);

  /// The time derivative's contribution to the right hand side of a linear system
  /// @param dof_id The dof index at which this contribution should be fetched at
  /// @param factors Multiplicative factor (e.g. a material property) at multiple
  ///                states (old, older, etc)
  virtual Real timeDerivativeRHSContribution(dof_id_type dof_id,
                                             const std::vector<Real> & factors = {}) const;

  /// The time derivative's contribution to the right hand side of a linear system.
  /// For now, this does not depend of the DoF index, might change in the future.
  virtual Real timeDerivativeMatrixContribution(const Real factor) const;

protected:
  /// Pointer to the linear system, can happen that we dont have any
  LinearSystem * _linear_system;

  /// Nonlinear implicit system, if applicable; otherwise, nullptr
  libMesh::LinearImplicitSystem * _linear_implicit_system;
};
