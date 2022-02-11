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

class ADOneDEnergyWallHeatFlux : public ADKernel
{
public:
  ADOneDEnergyWallHeatFlux(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  /// Heat flux
  const ADMaterialProperty<Real> & _q_wall;
  /// Heat flux perimeter
  const ADVariableValue & _P_hf;

public:
  static InputParameters validParams();
};
