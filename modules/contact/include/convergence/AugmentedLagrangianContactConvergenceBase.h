//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InputParameters.h"

/**
 * Non-templated base of AugmentedLagrangianContactConvergence that owns the augmented Lagrangian
 * iteration state, so that it can be accessed without knowing the underlying convergence type.
 */
class AugmentedLagrangianContactConvergenceBase
{
public:
  AugmentedLagrangianContactConvergenceBase(const InputParameters & params)
    : _maximum_number_lagrangian_iterations(
          params.get<int>("maximum_lagrangian_update_iterations")),
      _lagrangian_iteration_number(0)
  {
  }

  /// @return the current augmented Lagrangian iteration number
  const unsigned int & getLagrangianIterationNumber() const { return _lagrangian_iteration_number; }

protected:
  /// maximum number of augmented Lagrangian iterations
  const unsigned int _maximum_number_lagrangian_iterations;

  /// current augmented Lagrangian iteration number
  unsigned int _lagrangian_iteration_number;
};
