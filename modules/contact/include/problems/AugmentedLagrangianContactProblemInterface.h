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
#include "AugmentedLagrangianContactConvergenceBase.h"

class FEProblemBase;

/**
 *  Class to provide an interface for parameters and routines required to check
 *  convergence for the augmented Lagrangian contact problem.
 */
class AugmentedLagrangianContactProblemInterface
{
public:
  static InputParameters validParams();

  AugmentedLagrangianContactProblemInterface(const InputParameters & params,
                                             FEProblemBase & problem);

  /// @return the augmented Lagrangian contact convergence object of the problem's nonlinear solve
  virtual const AugmentedLagrangianContactConvergenceBase &
  getAugmentedLagrangianContactConvergence() const;

  virtual const unsigned int & getLagrangianIterationNumber() const
  {
    return getAugmentedLagrangianContactConvergence().getLagrangianIterationNumber();
  }

private:
  /// The problem that owns this interface, used to look up the convergence object
  FEProblemBase & _problem;
};
