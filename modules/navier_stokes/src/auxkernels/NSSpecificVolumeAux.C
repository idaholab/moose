/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSSpecificVolumeAux.h"
#include "MooseMesh.h"

template<>
InputParameters validParams<NSSpecificVolumeAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("rho", "density");

  return params;
}

NSSpecificVolumeAux::NSSpecificVolumeAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _rho(coupledValue("rho"))
{
}

Real
NSSpecificVolumeAux::computeValue()
{
  // Return a "big" value rather than dividing by zero.
  if (_rho[_qp] == 0.)
    return 1.e10;

  return 1. / _rho[_qp];
}
