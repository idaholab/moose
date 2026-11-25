//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

/**
 * Field kernel for the Keyes MSPIN test coupled source term.
 */
class ADCoupledFieldKernel : public ADKernelValue
{
public:
  static InputParameters validParams();
  ADCoupledFieldKernel(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  const ADVariableValue & _coupled_u;
  const ADVariableGradient & _grad_coupled_u;
};
