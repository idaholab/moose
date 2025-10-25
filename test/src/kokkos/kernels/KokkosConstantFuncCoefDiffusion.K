//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConstantFuncCoefDiffusion.h"

registerKokkosResidualObject("MooseTestApp", KokkosConstantFuncCoefDiffusion);

InputParameters
KokkosConstantFuncCoefDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<FunctionName>("coef", "1", "The function for conductivity");
  return params;
}

KokkosConstantFuncCoefDiffusion::KokkosConstantFuncCoefDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _function(getKokkosFunction<KokkosConstantFunction>("coef"))
{
}
