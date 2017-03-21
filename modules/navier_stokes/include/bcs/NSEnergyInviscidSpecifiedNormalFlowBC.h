/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSENERGYINVISCIDSPECIFIEDNORMALFLOWBC_H
#define NSENERGYINVISCIDSPECIFIEDNORMALFLOWBC_H

#include "NSEnergyInviscidBC.h"

// Forward Declarations
class NSEnergyInviscidSpecifiedNormalFlowBC;

template <>
InputParameters validParams<NSEnergyInviscidSpecifiedNormalFlowBC>();

/**
 * The inviscid energy BC term with specified normal flow.
 */
class NSEnergyInviscidSpecifiedNormalFlowBC : public NSEnergyInviscidBC
{
public:
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

#endif // NSENERGYINVISCIDSPECIFIEDNORMALFLOWBC_H
