//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVArrayElementalKernel.h"

class FVArrayTimeKernel : public FVArrayElementalKernel
{
public:
  static InputParameters validParams();
  FVArrayTimeKernel(const InputParameters & parameters);

protected:
  ADRealEigenVector computeQpResidual() override;

  const ADArrayVariableValue & _u_dot;
};
