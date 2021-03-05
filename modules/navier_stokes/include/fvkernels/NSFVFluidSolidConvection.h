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

/**
 * Kernel providing the convective heat transfer term in the fluid
 * energy equation, with strong form $\alpha\left(T_f-T_s\right)$.
 */
class NSFVFluidSolidConvection : public FVElementalKernel
{
public:
  NSFVFluidSolidConvection(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  ADReal computeQpResidual() override;

  /// solid temperature
  const ADVariableValue & _T_solid;

  /// fluid temperature
  const ADMaterialProperty<Real> & _T_fluid;

  /// interphase heat transfer coefficient
  const ADMaterialProperty<Real> & _alpha;
};
