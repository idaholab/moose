/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NSMASSINVISCIDFLUX_H
#define NSMASSINVISCIDFLUX_H

#include "NSKernel.h"

// Forward Declarations
class NSMassInviscidFlux;

template <>
InputParameters validParams<NSMassInviscidFlux>();

class NSMassInviscidFlux : public NSKernel
{
public:
  NSMassInviscidFlux(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
};

#endif // NSMASSINVISCIDFLUX_H
