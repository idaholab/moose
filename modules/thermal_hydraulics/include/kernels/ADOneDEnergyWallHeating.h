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

// The spatial part of the 1D energy conservation for Navier-Stokes flow
class ADOneDEnergyWallHeating : public ADKernel
{
public:
  ADOneDEnergyWallHeating(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  const ADMaterialProperty<Real> & _temperature;
  const ADMaterialProperty<Real> & _Hw;

  const ADVariableValue & _T_wall;
  const ADVariableValue & _P_hf;

public:
  static InputParameters validParams();
};
