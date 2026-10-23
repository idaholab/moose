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
 *  Class to provide an interface for parameters and routines required to check
 *  convergence for the augmented Lagrangian contact problem.
 */
class AugmentedLagrangianContactProblemInterface
{
public:
  static InputParameters validParams();

  AugmentedLagrangianContactProblemInterface(const InputParameters & params);

  /// @return the current augmented Lagrangian iteration number
  virtual const unsigned int & getLagrangianIterationNumber() const
  {
    return _lagrangian_iteration_number;
  }

protected:
  /// maximum number of augmented Lagrangian iterations
  const unsigned int _maximum_number_lagrangian_iterations;

  /// current augmented Lagrangian iteration number
  unsigned int _lagrangian_iteration_number;
};
