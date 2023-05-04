//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRadiativeHeatFluxBC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADRadiativeHeatFluxBC);

InputParameters
ADRadiativeHeatFluxBC::validParams()
{
  InputParameters params = ADRadiativeHeatFluxBCBase::validParams();

  params.addParam<FunctionName>("view_factor", "1", "View factor function");
  params.addDeprecatedParam<PostprocessorName>(
      "scale_pp",
      "1.0",
      "Post-processor by which to scale boundary condition",
      "The 'scale' parameter is replacing the 'scale_pp' parameter. 'scale' is a function "
      "parameter instead of a post-processor parameter. If you need to scale from a post-processor "
      "value, use a PostprocessorFunction.");
  params.addParam<FunctionName>("scale", 1.0, "Function by which to scale the boundary condition");

  params.addClassDescription(
      "Radiative heat transfer boundary condition for a plate heat structure");

  return params;
}

ADRadiativeHeatFluxBC::ADRadiativeHeatFluxBC(const InputParameters & parameters)
  : ADRadiativeHeatFluxBCBase(parameters),

    _view_factor_fn(getFunction("view_factor")),
    _scale_pp(getPostprocessorValue("scale_pp")),
    _scale_fn(getFunction("scale"))
{
}

ADReal
ADRadiativeHeatFluxBC::coefficient() const
{
  return _scale_pp * _scale_fn.value(_t, _q_point[_qp]) * _eps_boundary *
         _view_factor_fn.value(_t, _q_point[_qp]);
}
