//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleConvection.h"

registerMooseObject("ExampleApp", ExampleConvection);

InputParameters
ExampleConvection::validParams()
{
  InputParameters params = Kernel::validParams();

  params.addRequiredCoupledVar(
      "some_variable", "The gradient of this variable will be used as the velocity vector.");
  return params;
}

ExampleConvection::ExampleConvection(const InputParameters & parameters)
  : Kernel(parameters), _some_variable(coupledGradient("some_variable"))
{
}

Real
ExampleConvection::computeQpResidual()
{
  return _test[_i][_qp] * (_some_variable[_qp] * _grad_u[_qp]);
}

Real
ExampleConvection::computeQpJacobian()
{
  return _test[_i][_qp] * (_some_variable[_qp] * _grad_phi[_j][_qp]);
}
