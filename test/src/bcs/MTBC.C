//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MTBC.h"

registerMooseObject("MooseTestApp", MTBC);

InputParameters
MTBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "the name of the material property we are going to use");
  params.addRequiredParam<Real>("grad", "the value of the gradient");
  return params;
}

MTBC::MTBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _value(getParam<Real>("grad")),
    _mat(getMaterialProperty<Real>("prop_name"))
{
}

Real
MTBC::computeQpResidual()
{
  return -_test[_i][_qp] * _value * _mat[_qp];
}
