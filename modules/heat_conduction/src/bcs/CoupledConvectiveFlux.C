//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledConvectiveFlux.h"

#include "Function.h"

registerMooseObject("HeatConductionApp", CoupledConvectiveFlux);

InputParameters
CoupledConvectiveFlux::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredCoupledVar("T_infinity", "Field holding far-field temperature");
  params.addRequiredParam<Real>("coefficient", "Heat transfer coefficient");

  return params;
}

CoupledConvectiveFlux::CoupledConvectiveFlux(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _T_infinity(coupledValue("T_infinity")),
    _coefficient(getParam<Real>("coefficient"))
{
  mooseDeprecated(
      "CoupledConvectiveFlux boundary condition is deprecated, use CoupledConvectiveHeatFluxBC "
      "instead. To update your input file:\n  1. change type from `CoupledConvectiveFlux` to "
      "`CoupledConvectiveHeatFluxBC`\n  2. change `coefficient` parameter to `htc`.");
}

Real
CoupledConvectiveFlux::computeQpResidual()
{
  return _test[_i][_qp] * _coefficient * (_u[_qp] - _T_infinity[_qp]);
}

Real
CoupledConvectiveFlux::computeQpJacobian()
{
  return _test[_i][_qp] * _coefficient * _phi[_j][_qp];
}
