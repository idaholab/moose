//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrayLambertRadiationHeatFluxAux.h"
#include "GrayLambertSurfaceRadiationBase.h"

registerMooseObject("HeatTransferApp", GrayLambertRadiationHeatFluxAux);

InputParameters
GrayLambertRadiationHeatFluxAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Radiation heat flux from a GrayLambertSurfaceRadiationBase object.");
  params.addRequiredParam<UserObjectName>("surface_radiation_object",
                                          "GrayLambertSurfaceRadiationBase UO name");
  return params;
}

GrayLambertRadiationHeatFluxAux::GrayLambertRadiationHeatFluxAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _glsr_uo(getUserObject<GrayLambertSurfaceRadiationBase>("surface_radiation_object"))
{
  if (isNodal())
    mooseWarning("It is recommended to use a 'CONSTANT MONOMIAL' variable to avoid ambiguous "
                 "definition at nodes shared by sidesets.");
}

Real
GrayLambertRadiationHeatFluxAux::computeValue()
{
  return -_glsr_uo.getSurfaceHeatFluxDensity(_current_boundary_id);
}
