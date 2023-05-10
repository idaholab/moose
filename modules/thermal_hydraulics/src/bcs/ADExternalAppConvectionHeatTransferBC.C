//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADExternalAppConvectionHeatTransferBC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADExternalAppConvectionHeatTransferBC);

InputParameters
ADExternalAppConvectionHeatTransferBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();

  params.addRequiredCoupledVar("T_ext", "Temperature from external application");
  params.addRequiredCoupledVar("htc_ext", "Heat transfer coefficient from external application");
  params.addDeprecatedParam<PostprocessorName>(
      "scale_pp",
      "1.0",
      "Post-processor by which to scale boundary condition",
      "The 'scale' parameter is replacing the 'scale_pp' parameter. 'scale' is a function "
      "parameter instead of a post-processor parameter. If you need to scale from a post-processor "
      "value, use a PostprocessorFunction.");
  params.addParam<FunctionName>("scale", 1.0, "Function by which to scale the boundary condition");

  params.addClassDescription("Convection BC from an external application");

  return params;
}

ADExternalAppConvectionHeatTransferBC::ADExternalAppConvectionHeatTransferBC(
    const InputParameters & parameters)
  : ADIntegratedBC(parameters),

    _T_ext(adCoupledValue("T_ext")),
    _htc_ext(adCoupledValue("htc_ext")),
    _scale_pp(getPostprocessorValue("scale_pp")),
    _scale_fn(getFunction("scale"))
{
}

ADReal
ADExternalAppConvectionHeatTransferBC::computeQpResidual()
{
  return _scale_pp * _scale_fn.value(_t, _q_point[_qp]) * _htc_ext[_qp] * (_u[_qp] - _T_ext[_qp]) *
         _test[_i][_qp];
}
