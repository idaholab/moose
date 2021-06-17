//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SpecificVolumeAux.h"

registerMooseObject("NavierStokesApp", SpecificVolumeAux);

InputParameters
SpecificVolumeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredCoupledVar("rho", "Density of the phase");
  params.addCoupledVar("area", 1., "Cross-sectional area (if used)");
  params.addCoupledVar("alpha", 1., "Volume fraction (if used)");
  params.addClassDescription("This auxkernel computes the specific volume $v$ of the fluid.");

  return params;
}

SpecificVolumeAux::SpecificVolumeAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rho(coupledValue("rho")),
    _area(coupledValue("area")),
    _alpha(coupledValue("alpha"))
{
}

Real
SpecificVolumeAux::computeValue()
{
  mooseAssert(_rho[_qp] != 0, "Detected zero density.");
  return _alpha[_qp] * _area[_qp] / _rho[_qp];
}
