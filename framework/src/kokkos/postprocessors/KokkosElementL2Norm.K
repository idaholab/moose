//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosElementL2Norm.h"

registerKokkosPostprocessor("MooseApp", KokkosElementL2Norm);

InputParameters
KokkosElementL2Norm::validParams()
{
  InputParameters params = KokkosElementIntegralVariablePostprocessor::validParams();
  return params;
}

KokkosElementL2Norm::KokkosElementL2Norm(const InputParameters & parameters)
  : KokkosElementIntegralVariablePostprocessor(parameters)
{
}

Real
KokkosElementL2Norm::getValue() const
{
  return std::sqrt(KokkosElementIntegralVariablePostprocessor::getValue());
}
