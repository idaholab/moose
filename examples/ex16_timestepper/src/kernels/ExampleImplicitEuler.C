//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleImplicitEuler.h"

#include "Material.h"

registerMooseObject("ExampleApp", ExampleImplicitEuler);

InputParameters
ExampleImplicitEuler::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  return params;
}

ExampleImplicitEuler::ExampleImplicitEuler(const InputParameters & parameters)
  : TimeDerivative(parameters), _time_coefficient(getMaterialProperty<Real>("time_coefficient"))
{
}

Real
ExampleImplicitEuler::computeQpResidual()
{
  return _time_coefficient[_qp] * TimeDerivative::computeQpResidual();
}

Real
ExampleImplicitEuler::computeQpJacobian()
{
  return _time_coefficient[_qp] * TimeDerivative::computeQpJacobian();
}
