//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class OneDEnergyWallHeatFlux : public Kernel
{
public:
  OneDEnergyWallHeatFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Heat flux
  const MaterialProperty<Real> & _q_wall;
  /// Heat flux perimeter
  const VariableValue & _P_hf;

public:
  static InputParameters validParams();
};
