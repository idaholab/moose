/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Navier-Stokes includes
#include "NSSpecificVolumeAux.h"
#include "NS.h"

// MOOSE includes
#include "MooseMesh.h"

template <>
InputParameters
validParams<NSSpecificVolumeAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription(
      "Auxiliary kernel for computing the specific volume (1/rho) of the fluid.");
  params.addRequiredCoupledVar(NS::density, "density");

  return params;
}

NSSpecificVolumeAux::NSSpecificVolumeAux(const InputParameters & parameters)
  : AuxKernel(parameters), _rho(coupledValue(NS::density))
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
