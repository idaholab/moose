//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADConvectionHeatTransferBC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADConvectionHeatTransferBC);

InputParameters
ADConvectionHeatTransferBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<FunctionName>("T_ambient", "Ambient temperature function");
  params.addRequiredParam<FunctionName>("htc_ambient",
                                        "Ambient heat transfer coefficient function");
  params.addDeprecatedParam<PostprocessorName>(
      "scale_pp",
      "1.0",
      "Post-processor by which to scale boundary condition",
      "The 'scale' parameter is replacing the 'scale_pp' parameter. 'scale' is a function "
      "parameter instead of a post-processor parameter. If you need to scale from a post-processor "
      "value, use a PostprocessorFunction.");
  params.addParam<FunctionName>("scale", 1.0, "Function by which to scale the boundary condition");
  return params;
}

ADConvectionHeatTransferBC::ADConvectionHeatTransferBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _T_ambient_fn(getFunction("T_ambient")),
    _htc_ambient_fn(getFunction("htc_ambient")),
    _scale_pp(getPostprocessorValue("scale_pp")),
    _scale_fn(getFunction("scale"))
{
}

ADReal
ADConvectionHeatTransferBC::computeQpResidual()
{
  return _scale_pp * _scale_fn.value(_t, _q_point[_qp]) * _htc_ambient_fn.value(_t, _q_point[_qp]) *
         (_u[_qp] - _T_ambient_fn.value(_t, _q_point[_qp])) * _test[_i][_qp];
}
