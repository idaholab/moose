//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "INSTemperatureNoBCBC.h"

registerMooseObject("NavierStokesApp", INSTemperatureNoBCBC);

InputParameters
INSTemperatureNoBCBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();

  params.addClassDescription("This class implements the 'No BC' boundary condition discussed by "
                             "Griffiths, Papanastiou, and others.");
  // Required parameters
  params.addParam<MaterialPropertyName>("k_name", "k", "thermal conductivity_name");

  return params;
}

INSTemperatureNoBCBC::INSTemperatureNoBCBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    // Material property
    _k(getMaterialProperty<Real>("k_name"))
{
}

Real
INSTemperatureNoBCBC::computeQpResidual()
{
  // k * (grad_T.n) * test
  return _k[_qp] * _grad_u[_qp] * _normals[_qp] * _test[_i][_qp];
}

Real
INSTemperatureNoBCBC::computeQpJacobian()
{
  return _k[_qp] * (_grad_phi[_j][_qp] * _normals[_qp]) * _test[_i][_qp];
}

Real
INSTemperatureNoBCBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // off-diagonal derivatives are all zero.
  return 0.;
}
