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
 * The inviscid energy BC term with specified pressure.
 */
class NSEnergyInviscidSpecifiedPressureBC : public NSEnergyInviscidBC
{
public:
  static InputParameters validParams();

  NSEnergyInviscidSpecifiedPressureBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Required parameters
  Real _specified_pressure;

private:
  // Helper Jacobian function
  Real computeJacobianHelper(unsigned var_number);
};
