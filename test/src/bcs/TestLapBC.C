//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestLapBC.h"

registerMooseObject("MooseTestApp", TestLapBC);

InputParameters
TestLapBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  return params;
}

TestLapBC::TestLapBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _second_u(_var.secondSln()), _second_phi(_var.secondPhiFace())
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
