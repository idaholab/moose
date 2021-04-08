//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernelValue.h"

class ADMaterialPropertyValue : public ADKernelValue
{
public:
  static InputParameters validParams();

  ADMaterialPropertyValue(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  const Real _kernel_sign;
  const ADMaterialProperty<Real> & _prop;
};
