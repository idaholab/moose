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
#include "DerivativeMaterialInterfaceTHM.h"

// The spatial part of the 1D energy conservation for Navier-Stokes flow
class OneDEnergyWallHeating : public DerivativeMaterialInterfaceTHM<Kernel>
{
public:
  OneDEnergyWallHeating(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const MaterialProperty<Real> & _temperature;
  const MaterialProperty<Real> & _dT_drhoA;
  const MaterialProperty<Real> & _dT_drhouA;
  const MaterialProperty<Real> & _dT_drhoEA;
  const MaterialProperty<Real> & _Hw;

  const VariableValue & _T_wall;
  const VariableValue & _P_hf;

  // For Jacobian terms
  unsigned _rhoA_var_number;
  unsigned _rhouA_var_number;

public:
  static InputParameters validParams();
};
