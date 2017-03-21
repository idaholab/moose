/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "RobinBC.h"

template <>
InputParameters
validParams<RobinBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addParam<Real>("value", 0.0, "The value of the gradient on the boundary.");
  return params;
}

RobinBC::RobinBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _value(parameters.get<Real>("value"))
{
}

Real
RobinBC::computeQpResidual()
{
  return (_grad_u[_qp])(1) * _test[_i][_qp] + _u[_qp] - _value;
}
