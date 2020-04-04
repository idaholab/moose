//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionGradientNeumannBC.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionGradientNeumannBC);

InputParameters
FunctionGradientNeumannBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<FunctionName>("exact_solution", "The exact solution.");
  params.addParam<Real>("coeff", 1, "The diffusion, thermal conductivity, etc. coefficient");
  params.addClassDescription("Imposes the integrated boundary condition "
                             "arising from integration by parts of a diffusion/heat conduction "
                             "operator, and where the exact solution can be specified.");
  return params;
}

FunctionGradientNeumannBC::FunctionGradientNeumannBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _exact_solution(getFunction("exact_solution")),
    _coeff(getParam<Real>("coeff"))
{
}

Real
FunctionGradientNeumannBC::computeQpResidual()
{
  return -_test[_i][_qp] * _normals[_qp] * _coeff * _exact_solution.gradient(_t, _q_point[_qp]);
}
