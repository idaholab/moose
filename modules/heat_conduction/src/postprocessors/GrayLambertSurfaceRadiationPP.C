//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrayLambertSurfaceRadiationPP.h"

registerMooseObject("HeatConductionApp", GrayLambertSurfaceRadiationPP);

InputParameters
GrayLambertSurfaceRadiationPP::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  MooseEnum return_type("RADIOSITY HEAT_FLUX_DENSITY TEMPERATURE", "HEAT_FLUX_DENSITY");
  params.addParam<MooseEnum>("return_type",
                             return_type,
                             "Requested return type: RADIOSITY | HEAT_FLUX_DENSITY | TEMPERATURE");
  params.addRequiredParam<UserObjectName>("surface_radiation_object_name",
                                          "Name of the GrayLambertSurfaceRadiationBase UO");
  params.addRequiredParam<BoundaryName>("boundary", "The boundary of interest.");
  params.addClassDescription("This postprocessor allows to extract radiosity, heat flux density, "
                             "and temperature from the GrayLambertSurfaceRadiationBase object.");
  return params;
}

GrayLambertSurfaceRadiationPP::GrayLambertSurfaceRadiationPP(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _glsr_uo(getUserObject<GrayLambertSurfaceRadiationBase>("surface_radiation_object_name")),
    _return_type(getParam<MooseEnum>("return_type")),
    _bnd_id(_fe_problem.mesh().getBoundaryID(getParam<BoundaryName>("boundary")))
{
}

PostprocessorValue
GrayLambertSurfaceRadiationPP::getValue()
{
  if (_return_type == "RADIOSITY")
    return _glsr_uo.getSurfaceRadiosity(_bnd_id);
  else if (_return_type == "TEMPERATURE")
    return _glsr_uo.getSurfaceTemperature(_bnd_id);
  return _glsr_uo.getSurfaceHeatFluxDensity(_bnd_id);
}
