//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FuncDiffusion.h"
#include "Function.h"

registerMooseObject("ElectromagneticsApp", FuncDiffusion);

InputParameters
FuncDiffusion::validParams()
{
  InputParameters params = Diffusion::validParams();
  params.addClassDescription("The Laplacian operator with a function coefficient.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for diffusion term.");
  return params;
}

FuncDiffusion::FuncDiffusion(const InputParameters & parameters)
  : Diffusion(parameters),

    _func(getFunction("func"))

{
}

Real
FuncDiffusion::computeQpResidual()
{
  return _func.value(_t, _q_point[_qp]) * Diffusion::computeQpResidual();
}

Real
FuncDiffusion::computeQpJacobian()
{
  return _func.value(_t, _q_point[_qp]) * Diffusion::computeQpJacobian();
}
