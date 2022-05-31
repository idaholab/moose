//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionDiffusion.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionDiffusion);

InputParameters
FunctionDiffusion::validParams()
{
  InputParameters params = Diffusion::validParams();
  params.addClassDescription("The Laplacian operator with a function coefficient.");
  params.addParam<FunctionName>("function", 1.0, "Function multiplier for diffusion term.");
  return params;
}

FunctionDiffusion::FunctionDiffusion(const InputParameters & parameters)
  : Diffusion(parameters), _function(getFunction("function"))
{
}

Real
FunctionDiffusion::computeQpResidual()
{
  return _function.value(_t, _q_point[_qp]) * Diffusion::computeQpResidual();
}

Real
FunctionDiffusion::computeQpJacobian()
{
  return _function.value(_t, _q_point[_qp]) * Diffusion::computeQpJacobian();
}
