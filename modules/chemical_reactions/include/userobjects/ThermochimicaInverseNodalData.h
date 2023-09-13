//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThermochimicaNodalBase.h"
#include "NestedSolve.h"

/**
 * User object that performs a Newton interation to determine the composition needed to match a
 * given elemental chemical potential utilizing Gibbs energy minimization at each node by calling
 * the Thermochimica code.
 */
class ThermochimicaInverseNodalData : public ThermochimicaNodalBase
{
public:
  static InputParameters validParams();
  ThermochimicaInverseNodalData(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// Instantiation of the NestedSolve class
  NestedSolve _nested_solve;

  /// Writable elemental variable
  std::vector<MooseVariable *> _el_writable;

  /// Chemical potential to solve for
  const VariableValue & _mu;

  /// Index of which mu to match
  unsigned int _mu_index;

  /// Width for finite difference calculation
  const Real _finite_difference_width;

  /// Verbose flag
  const bool _verbose;
};
