//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosFuncCoefDiffusion.h"

registerKokkosResidualObject("MooseTestApp", KokkosFuncCoefDiffusion);

InputParameters
KokkosFuncCoefDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<FunctionName>("coef", "1", "The function for conductivity");
  return params;
}

KokkosFuncCoefDiffusion::KokkosFuncCoefDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _function(getKokkosFunction("coef"))
{
}
