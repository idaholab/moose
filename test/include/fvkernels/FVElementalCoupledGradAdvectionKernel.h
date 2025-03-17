//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/*
 *  FVElementalCoupledGradAdvectionKernel implements:
 *
 *      - strong form: \nabla v \cdot \nabla u
 *
 *      - weak form: \int_{V} \nabla v \cdot \nabla u dV
 */
class FVElementalCoupledGradAdvectionKernel : public FVElementalKernel
{
public:
  static InputParameters validParams();
  FVElementalCoupledGradAdvectionKernel(const InputParameters & params);

protected:
  virtual ADReal computeQpResidual() override;

  /// Gradient of coupled variable (either FE or FV)
  const ADVariableGradient & _grad_v;
  /// Gradient of primary variable
  const ADVariableGradient & _grad_u;
};
