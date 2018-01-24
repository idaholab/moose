/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ExampleConvection.h"

template <>
InputParameters
validParams<ExampleConvection>()
{
  InputParameters params = validParams<Kernel>();

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
