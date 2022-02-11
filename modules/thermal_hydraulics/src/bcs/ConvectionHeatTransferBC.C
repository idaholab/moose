//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvectionHeatTransferBC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ConvectionHeatTransferBC);

InputParameters
ConvectionHeatTransferBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<FunctionName>("T_ambient", "Ambient temperature function");
  params.addRequiredParam<FunctionName>("htc_ambient",
                                        "Ambient heat transfer coefficient function");
  params.addParam<PostprocessorName>(
      "scale_pp", 1.0, "Post-processor by which to scale boundary condition");
  return params;
}

ConvectionHeatTransferBC::ConvectionHeatTransferBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _T_ambient_fn(getFunction("T_ambient")),
    _htc_ambient_fn(getFunction("htc_ambient")),
    _scale_pp(getPostprocessorValue("scale_pp"))
{
}

Real
ConvectionHeatTransferBC::computeQpResidual()
{
  return _scale_pp * _htc_ambient_fn.value(_t, _q_point[_qp]) *
         (_u[_qp] - _T_ambient_fn.value(_t, _q_point[_qp])) * _test[_i][_qp];
}

Real
ConvectionHeatTransferBC::computeQpJacobian()
{
  return _scale_pp * _htc_ambient_fn.value(_t, _q_point[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
}
