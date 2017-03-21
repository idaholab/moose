/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CoupledConvectiveFlux.h"

#include "Function.h"

template <>
InputParameters
validParams<CoupledConvectiveFlux>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredCoupledVar("T_infinity", "Field holding far-field temperature");
  params.addRequiredParam<Real>("coefficient", "Heat transfer coefficient");

  return params;
}

CoupledConvectiveFlux::CoupledConvectiveFlux(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _T_infinity(coupledValue("T_infinity")),
    _coefficient(getParam<Real>("coefficient"))
{
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
