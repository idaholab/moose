//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OneDNodalBC.h"
#include "OneDStagnationPandTBase.h"

class SinglePhaseFluidProperties;

/**
 * Stagnation P and T BC
 */
class OneDEnergyStagnationPandTBC : public OneDNodalBC, public OneDStagnationPandTBase
{
public:
  OneDEnergyStagnationPandTBC(const InputParameters & parameters);

protected:
  virtual bool shouldApply();
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  const bool & _reversible;
  const Real & _T0;
  const Real & _p0;

  const VariableValue & _area;
  const VariableValue & _vel;
  const VariableValue & _vel_old;
  const VariableValue & _arhoA;
  const VariableValue & _arhouA;
  const VariableValue & _arhoEA;

  const unsigned int _arhoA_var_number;
  const unsigned int _arhouA_var_number;

public:
  static InputParameters validParams();
};
