//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExternalAppConvectionHeatTransferBC.h"

registerMooseObject("ThermalHydraulicsApp", ExternalAppConvectionHeatTransferBC);

InputParameters
ExternalAppConvectionHeatTransferBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();

  params.addRequiredCoupledVar("T_ext", "Temperature from external application");
  params.addRequiredCoupledVar("htc_ext", "Heat transfer coefficient from external application");
  params.addParam<PostprocessorName>(
      "scale_pp", 1.0, "Post-processor by which to scale boundary condition");

  params.addClassDescription("Convection BC from an external application");

  return params;
}

ExternalAppConvectionHeatTransferBC::ExternalAppConvectionHeatTransferBC(
    const InputParameters & parameters)
  : IntegratedBC(parameters),

    _T_ext(coupledValue("T_ext")),
    _htc_ext(coupledValue("htc_ext")),
    _scale_pp(getPostprocessorValue("scale_pp"))
{
}

Real
ExternalAppConvectionHeatTransferBC::computeQpResidual()
{
  return _scale_pp * _htc_ext[_qp] * (_u[_qp] - _T_ext[_qp]) * _test[_i][_qp];
}

Real
ExternalAppConvectionHeatTransferBC::computeQpJacobian()
{
  return _scale_pp * _htc_ext[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}
