//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHSHeatFluxBC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADHSHeatFluxBC);

InputParameters
ADHSHeatFluxBC::validParams()
{
  InputParameters params = ADFunctionNeumannBC::validParams();

  params.addDeprecatedParam<PostprocessorName>(
      "scale_pp",
      "1.0",
      "Post-processor by which to scale boundary condition",
      "The 'scale' parameter is replacing the 'scale_pp' parameter. 'scale' is a function "
      "parameter instead of a post-processor parameter. If you need to scale from a post-processor "
      "value, use a PostprocessorFunction.");
  params.addParam<FunctionName>("scale", 1.0, "Function by which to scale the boundary condition");

  params.addClassDescription("Applies a specified heat flux to the side of a plate heat structure");

  return params;
}

ADHSHeatFluxBC::ADHSHeatFluxBC(const InputParameters & parameters)
  : ADFunctionNeumannBC(parameters),

    _scale_pp(getPostprocessorValue("scale_pp")),
    _scale_fn(getFunction("scale"))
{
}

ADReal
ADHSHeatFluxBC::computeQpResidual()
{
  return _scale_pp * _scale_fn.value(_t, _q_point[_qp]) * ADFunctionNeumannBC::computeQpResidual();
}
