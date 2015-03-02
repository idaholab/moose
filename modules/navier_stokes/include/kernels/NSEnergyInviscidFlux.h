/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSENERGYINVISCIDFLUX_H
#define NSENERGYINVISCIDFLUX_H

#include "NSKernel.h"

// Forward Declarations
class NSEnergyInviscidFlux;

template<>
InputParameters validParams<NSEnergyInviscidFlux>();

class NSEnergyInviscidFlux : public NSKernel
{
public:

  NSEnergyInviscidFlux(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Coupled variables
  VariableValue & _enthalpy;
};

#endif
