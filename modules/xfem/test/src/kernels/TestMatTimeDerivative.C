//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestMatTimeDerivative.h"

registerMooseObject("XFEMTestApp", TestMatTimeDerivative);

InputParameters
TestMatTimeDerivative::validParams()
{
  InputParameters params = TimeDerivative::validParams();
  params.addRequiredParam<MaterialPropertyName>("mat_prop_value",
                                                "Material "
                                                "property to multiply by time "
                                                "derivative");
  return params;
}

TestMatTimeDerivative::TestMatTimeDerivative(const InputParameters & parameters)
  : TimeDerivative(parameters), _mat_prop_value(getMaterialProperty<Real>("mat_prop_value"))
{
}

Real
TestMatTimeDerivative::computeQpResidual()
{
  return _mat_prop_value[_qp] * TimeDerivative::computeQpResidual();
}

Real
TestMatTimeDerivative::computeQpJacobian()
{
  return _mat_prop_value[_qp] * TimeDerivative::computeQpJacobian();
}
