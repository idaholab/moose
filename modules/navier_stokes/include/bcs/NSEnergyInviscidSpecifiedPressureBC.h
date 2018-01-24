/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSENERGYINVISCIDSPECIFIEDPRESSUREBC_H
#define NSENERGYINVISCIDSPECIFIEDPRESSUREBC_H

#include "NSEnergyInviscidBC.h"

// Forward Declarations
class NSEnergyInviscidSpecifiedPressureBC;

template <>
InputParameters validParams<NSEnergyInviscidSpecifiedPressureBC>();

/**
 * The inviscid energy BC term with specified pressure.
 */
class NSEnergyInviscidSpecifiedPressureBC : public NSEnergyInviscidBC
{
public:
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

#endif // NSENERGYINVISCIDSPECIFIEDPRESSUREBC_H
