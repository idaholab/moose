//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ValueTest.h"

registerMooseObject("MooseTestApp", ValueTest);

InputParameters
ValueTest::validParams()
{
  return Kernel::validParams();
}

ValueTest::ValueTest(const InputParameters & parameters) : Kernel(parameters) {}

Real
ValueTest::computeQpResidual()
{
  return -_u[_qp] * _test[_i][_qp];
}

Real
ValueTest::computeQpJacobian()
{
  return -_phi[_j][_qp] * _test[_i][_qp];
}
