//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
