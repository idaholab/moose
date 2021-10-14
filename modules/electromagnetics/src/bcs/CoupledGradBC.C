//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledGradBC.h"
#include "Function.h"

registerMooseObject("ElectromagneticsApp", CoupledGradBC);

InputParameters
CoupledGradBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addClassDescription(
      "Imposes the integrated boundary condition arising from integration by parts of a Helmholtz "
      "equation, when that term is set equal to the gradient of a coupled variable. ");
  params.addParam<FunctionName>(
      "func", 1.0, "Optional function coefficient for coupled gradient term.");
  params.addRequiredCoupledVar("coupled_field", "Coupled field variable.");
  return params;
}

CoupledGradBC::CoupledGradBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters),
    _sign(getParam<Real>("sign")),
    _coefficient(getParam<Real>("coefficient")),
    _func(getFunction("func")),
    _coupled_grad(adCoupledGradient("coupled_field"))
{
}

ADReal
CoupledGradBC::computeQpResidual()
{
  return _func.value(_t, _q_point[_qp]) * _coupled_grad[_qp] * _normals[_qp];
}
