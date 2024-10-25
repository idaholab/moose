//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  virtual const unsigned int & getLagrangianIterationNumber() const
  {
    return _lagrangian_iteration_number;
  }
  virtual void setLagrangianIterationNumber(unsigned int iter)
  {
    _lagrangian_iteration_number = iter;
  }

protected:
  /// maximum mumber of augmented lagrange iterations
  const unsigned int _maximum_number_lagrangian_iterations;

  /// current augmented lagrange iteration number
  unsigned int _lagrangian_iteration_number;
};
