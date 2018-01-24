//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NSENERGYINVISCIDSPECIFIEDBC_H
#define NSENERGYINVISCIDSPECIFIEDBC_H

#include "NSEnergyInviscidBC.h"

// Forward Declarations
class NSEnergyInviscidSpecifiedBC;

template <>
InputParameters validParams<NSEnergyInviscidSpecifiedBC>();

/**
 * The inviscid energy BC term with specified pressure.
 */
class NSEnergyInviscidSpecifiedBC : public NSEnergyInviscidBC
{
public:
  NSEnergyInviscidSpecifiedBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Required parameters
  const Real _specified_pressure;
  const Real _un;

private:
  // Helper Jacobian function
  Real computeJacobianHelper(unsigned var_number);
};

#endif // NSENERGYINVISCIDSPECIFIEDBC_H
