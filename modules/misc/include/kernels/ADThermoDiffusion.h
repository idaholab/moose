//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

class ADThermoDiffusion : public ADKernel
{
public:
  static InputParameters validParams();

  ADThermoDiffusion(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  const ADVariableGradient & _grad_temp;
  const ADMaterialProperty<Real> & _soret_coeff;
};
