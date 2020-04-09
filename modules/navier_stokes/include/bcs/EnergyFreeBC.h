//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

/**
 *
 */
class EnergyFreeBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  EnergyFreeBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const VariableValue & _enthalpy;
  const VariableValue & _rho_u;
  const VariableValue & _rho_v;
  const VariableValue & _rho_w;
};
