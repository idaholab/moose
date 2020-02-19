//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvectionTest.h"

registerMooseObject("MooseTestApp", ConvectionTest);

InputParameters
ConvectionTest::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<RealVectorValue>("velocity", "Velocity Vector");
  params.addPrivateParam<std::vector<std::string>>("some_variable");
  return params;
}

ConvectionTest::ConvectionTest(const InputParameters & parameters)
  : Kernel(parameters), _velocity(getParam<RealVectorValue>("velocity"))
{
}

Real
ConvectionTest::computeQpResidual()
{
  return _test[_i][_qp] * (_velocity * _grad_u[_qp]);
}

Real
ConvectionTest::computeQpJacobian()
{
  return _test[_i][_qp] * (_velocity * _grad_phi[_j][_qp]);
}
