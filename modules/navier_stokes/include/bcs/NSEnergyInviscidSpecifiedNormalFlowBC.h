//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSEnergyInviscidBC.h"

// Forward Declarations

/**
 * The inviscid energy BC term with specified normal flow.
 */
class NSEnergyInviscidSpecifiedNormalFlowBC : public NSEnergyInviscidBC
{
public:
  static InputParameters validParams();

  NSEnergyInviscidSpecifiedNormalFlowBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Aux Variables
  const VariableValue & _pressure;

  // Required parameters
  const Real _un;

private:
  Real computeJacobianHelper(unsigned var_number);
};
