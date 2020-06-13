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
 * DG kernel for interfacing diffusion between two variables on adjacent blocks,
 * using the automatic differentiation system to calculate the Jacobian.
 */
class ADVectorCoupledInterfacialSource : public ADVectorInterfaceKernel
{
public:
  static InputParameters validParams();

  ADVectorCoupledInterfacialSource(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual(Moose::DGResidualType type) override;

  const MaterialProperty<Real> & _D;
  const MaterialProperty<Real> & _D_neighbor;

  const ADVectorVariableValue & _var_source;
  const ADVectorVariableValue & _var_source_neighbor;
};
