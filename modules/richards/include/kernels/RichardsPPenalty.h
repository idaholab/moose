//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward Declarations

/**
 * Kernel = a*(lower - variable) for variable<lower, and zero otherwise
 * This is an attempt to enforce variable>=lower
 */
class RichardsPPenalty : public Kernel
{
public:
  static InputParameters validParams();

  RichardsPPenalty(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  /// Kernel = a*(_lower - variable) for variable<lower and zero otherwise
  Real _a;

  /// Kernel = a*(_lower - variable) for variable<lower and zero otherwise
  const VariableValue & _lower;

  /// moose variable number of the _lower variable (needed for OffDiagJacobian)
  unsigned int _lower_var_num;
};
