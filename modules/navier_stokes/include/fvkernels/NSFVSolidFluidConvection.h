//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSFVFluidSolidConvection.h"

/**
 * kernel providing the convection term in the solid energy equation, with
 * strong form $\alpha(T_s-T_f)$.
 */
class NSFVSolidFluidConvection : public NSFVFluidSolidConvection
{
public:
  NSFVSolidFluidConvection(const InputParameters & parameters);
  static InputParameters validParams();

protected:
  ADReal computeQpResidual() override;
};
