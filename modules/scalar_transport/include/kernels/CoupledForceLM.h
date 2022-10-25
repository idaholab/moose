//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "LMKernel.h"

/**
 * Adds a coupled force term to a Lagrange multiplier constrained primal equation
 */
class CoupledForceLM : public LMKernel
{
public:
  CoupledForceLM(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  ADReal precomputeQpResidual() override;

  /// The number of the coupled variable
  const unsigned int _v_var;
  /// The current quadrature point values of the coupled variable
  const ADVariableValue & _v;
  /// An optional coefficient multiplying the coupled force
  const Real _coef;
};
