//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleConvection.h"

// Don't forget to register your object with MOOSE
registerMooseObject("ExampleApp", ExampleConvection);

InputParameters
ExampleConvection::validParams()
{
  InputParameters params = Kernel::validParams();

  // Here we specify a new parameter for our kernel allowing users to indicate which other
  // variable they want to be coupled into this kernel from an input file.
  params.addRequiredCoupledVar(
      "some_variable", "The gradient of this variable will be used as the velocity vector.");

  return params;
}

ExampleConvection::ExampleConvection(const InputParameters & parameters)
  : Kernel(parameters),
    // using the user-specified name for the coupled variable, retrieve and store a reference to the
    // coupled variable.
    _grad_some_variable(coupledGradient("some_variable"))
{
}

Real
ExampleConvection::computeQpResidual()
{
  // Implement the weak form equations using the coupled variable instead of the constant
  // parameter 'velocity' used in example 2.
  return _test[_i][_qp] * (_grad_some_variable[_qp] * _grad_u[_qp]);
}

Real
ExampleConvection::computeQpJacobian()
{
  // Implement the Jacobian using the coupled variable instead of the 'velocity'
  // constant parameter used in example 2.
  return _test[_i][_qp] * (_grad_some_variable[_qp] * _grad_phi[_j][_qp]);
}
