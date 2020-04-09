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

/**
 * A kernel for computing total energy convective flux
 *
 * \grad (\rho u H)
 *
 */
class TotalEnergyConvectiveFlux : public Kernel
{
public:
  static InputParameters validParams();

  TotalEnergyConvectiveFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  /// Density
  const VariableValue & _rho;
  /// Momentum
  const VariableValue & _rho_u;
  const VariableValue & _rho_v;
  const VariableValue & _rho_w;
  /// Enthalpy
  const VariableValue & _enthalpy;
};
