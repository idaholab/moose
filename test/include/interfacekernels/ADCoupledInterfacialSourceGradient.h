//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADInterfaceKernel.h"

/**
 * Diffusion with a source depending on the gradient jump of the coupled variable,
 * using the automatic differentiation system to calculate the Jacobian.
 */
class ADCoupledInterfacialSourceGradient : public ADInterfaceKernel
{
public:
  static InputParameters validParams();

  ADCoupledInterfacialSourceGradient(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  const MaterialProperty<Real> & _D;
  const MaterialProperty<Real> & _D_neighbor;

  const ADVariableGradient & _grad_var;
  const ADVariableGradient & _grad_var_neighbor;
};
