/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
