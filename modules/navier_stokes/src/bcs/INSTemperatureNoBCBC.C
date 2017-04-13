/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSTemperatureNoBCBC.h"

template <>
InputParameters
validParams<INSTemperatureNoBCBC>()
{
  InputParameters params = validParams<IntegratedBC>();

  params.addClassDescription("This class implements the 'No BC' boundary condition discussed by "
                             "Griffiths, Papanastiou, and others.");
  // Required parameters
  params.addRequiredParam<Real>("k", "thermal conductivity");

  return params;
}

INSTemperatureNoBCBC::INSTemperatureNoBCBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    // Required parameters
    _k(getParam<Real>("k"))
{
}

Real
INSTemperatureNoBCBC::computeQpResidual()
{
  // k * (grad_T.n) * test
  return _k * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp];
}

Real
INSTemperatureNoBCBC::computeQpJacobian()
{
  return _k * (_grad_phi[_j][_qp] * _normals[_qp]) * _test[_i][_qp];
}

Real
INSTemperatureNoBCBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // off-diagonal derivatives are all zero.
  return 0.;
}
