//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DiffusionVariableIntegral.h"

registerMooseObject("isopodApp", DiffusionVariableIntegral);

InputParameters
DiffusionVariableIntegral::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addClassDescription("computes some stuff for material gradient");

  params.addRequiredCoupledVar("variable1",
                               "The name of the variable that this object operates on");
  params.addRequiredCoupledVar("variable2", "Variable being multiplied");
  params.addRequiredParam<MaterialPropertyName>("material_derivative",
                                                "Material Derivative wrt parameterize");
  return params;
}

DiffusionVariableIntegral::DiffusionVariableIntegral(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _dMdP(getMaterialProperty<Real>("material_derivative")),
    _grad_u(coupledGradient("variable1")),
    _grad_v(coupledGradient("variable2"))
{
}

Real
DiffusionVariableIntegral::computeQpIntegral()
{
  return -_grad_v[_qp] * _dMdP[_qp] * _grad_u[_qp];
}
