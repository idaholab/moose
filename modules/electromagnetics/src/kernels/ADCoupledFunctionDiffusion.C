//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADCoupledFunctionDiffusion.h"
#include "Function.h"

registerMooseObject("ElectromagneticsApp", ADCoupledFunctionDiffusion);

InputParameters
ADCoupledFunctionDiffusion::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription(
      "Represents a coupled Laplacian term with sign and function coefficients.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for diffusion term.");
  MooseEnum sign("positive=1 negative=-1", "positive");
  params.addParam<MooseEnum>("sign", sign, "Sign of kernel.");
  params.addRequiredCoupledVar("coupled_field", "Coupled field variable.");
  return params;
}

ADCoupledFunctionDiffusion::ADCoupledFunctionDiffusion(const InputParameters & parameters)
  : ADKernel(parameters),
    _func(getFunction("func")),
    _sign(getParam<MooseEnum>("sign")),
    _coupled_grad(coupledGradient("coupled_field"))
{
}

ADReal
ADCoupledFunctionDiffusion::computeQpResidual()
{
  return _sign * _func.value(_t, _q_point[_qp]) * _grad_test[_i][_qp] * _coupled_grad[_qp];
}
