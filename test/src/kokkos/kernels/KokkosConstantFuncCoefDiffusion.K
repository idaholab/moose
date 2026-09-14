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
  // A numeric function name is built on demand as a KokkosParsedFunction, so a numeric default
  // cannot serve a kernel that asks for its function in a different concrete type
  params.addRequiredParam<FunctionName>("coef", "The function for conductivity");
  return params;
}

KokkosConstantFuncCoefDiffusion::KokkosConstantFuncCoefDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _function(getKokkosFunction<KokkosConstantFunction>("coef"))
{
}
