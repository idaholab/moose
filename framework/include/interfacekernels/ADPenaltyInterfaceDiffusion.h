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
 * DG kernel for interfacing diffusion between two variables on adjacent blocks
 */
class ADPenaltyInterfaceDiffusion : public ADInterfaceKernel
{
public:
  static InputParameters validParams();

  ADPenaltyInterfaceDiffusion(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::DGResidualType type) override;

  const Real _penalty;

  std::string _jump_prop_name;
  const ADMaterialProperty<Real> * const _jump;
};
