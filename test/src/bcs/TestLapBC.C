//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestLapBC.h"

template <>
InputParameters
validParams<TestLapBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  return params;
}

TestLapBC::TestLapBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _second_u(second()),
    _second_phi(secondPhiFace()),
    _second_test(secondTestFace())
{
}

Real
TestLapBC::computeQpResidual()
{
  return -0.5 * _second_u[_qp].tr() * _test[_i][_qp];
}

Real
TestLapBC::computeQpJacobian()
{
  return -0.5 * _second_phi[_j][_qp].tr() * _test[_i][_qp];
}
