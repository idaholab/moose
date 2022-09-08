//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GapFluxModelRadiation.h"

registerMooseObject("HeatConductionApp", GapFluxModelRadiation);

InputParameters
GapFluxModelRadiation::validParams()
{
  InputParameters params = GapFluxModelRadiationBase::validParams();
  params.addClassDescription(
      "Gap flux model for heat conduction across a gap due to radiation, "
      "based on the diffusion approximation. Uses a coupled temperature variable.");
  params.addRequiredCoupledVar("temperature", "The name of the temperature variable");
  return params;
}

GapFluxModelRadiation::GapFluxModelRadiation(const InputParameters & parameters)
  : GapFluxModelRadiationBase(parameters),
    _primary_T(adCoupledNeighborValue("temperature")),
    _secondary_T(adCoupledValue("temperature"))
{
}

ADReal
GapFluxModelRadiation::computeFlux() const
{
  return computeRadiationFlux(_secondary_T[_qp], _primary_T[_qp]);
}
