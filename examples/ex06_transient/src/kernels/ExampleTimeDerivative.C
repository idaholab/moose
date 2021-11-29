//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleTimeDerivative.h"

#include "Material.h"

registerMooseObject("ExampleApp", ExampleTimeDerivative);

InputParameters
ExampleTimeDerivative::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  params.addParam<Real>("time_coefficient", 1.0, "Time Coefficient");
  return params;
}

ExampleTimeDerivative::ExampleTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters), _time_coefficient(getParam<Real>("time_coefficient"))
{
}

Real
ExampleTimeDerivative::computeQpResidual()
{
  return _time_coefficient * TimeDerivative::computeQpResidual();
}

Real
ExampleTimeDerivative::computeQpJacobian()
{
  return _time_coefficient * TimeDerivative::computeQpJacobian();
}
