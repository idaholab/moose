/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSENERGYINVISCIDUNSPECIFIEDBC_H
#define NSENERGYINVISCIDUNSPECIFIEDBC_H

#include "NSEnergyInviscidBC.h"

// Forward Declarations
class NSEnergyInviscidUnspecifiedBC;

template <>
InputParameters validParams<NSEnergyInviscidUnspecifiedBC>();

/**
 * The inviscid energy BC term with specified pressure.
 */
class NSEnergyInviscidUnspecifiedBC : public NSEnergyInviscidBC
{
public:
  NSEnergyInviscidUnspecifiedBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned jvar);

  // Aux Variables
  const VariableValue & _pressure;

private:
  // Helper Jacobian function
  Real computeJacobianHelper(unsigned var_number);
};

#endif // NSENERGYINVISCIDUNSPECIFIEDBC_H
