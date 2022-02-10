//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunction1PhaseVelocityMagnitudeAux.h"

registerMooseObject("ThermalHydraulicsApp", VolumeJunction1PhaseVelocityMagnitudeAux);

InputParameters
VolumeJunction1PhaseVelocityMagnitudeAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredCoupledVar("rhoV", "rho*V of the junction");
  params.addRequiredCoupledVar("rhouV", "rho*u*V of the junction");
  params.addRequiredCoupledVar("rhovV", "rho*v*V of the junction");
  params.addRequiredCoupledVar("rhowV", "rho*w*V of the junction");
  params.addClassDescription(
      "Computes magnitude of velocity from the 1-phase volume junction variables.");
  return params;
}

VolumeJunction1PhaseVelocityMagnitudeAux::VolumeJunction1PhaseVelocityMagnitudeAux(
    const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _rhoV(coupledScalarValue("rhoV")),
    _rhouV(coupledScalarValue("rhouV")),
    _rhovV(coupledScalarValue("rhovV")),
    _rhowV(coupledScalarValue("rhowV"))
{
}

Real
VolumeJunction1PhaseVelocityMagnitudeAux::computeValue()
{
  const RealVectorValue vel(_rhouV[0] / _rhoV[0], _rhovV[0] / _rhoV[0], _rhowV[0] / _rhoV[0]);
  return vel.norm();
}
