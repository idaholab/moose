//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosElementExtremeValue.h"

registerKokkosPostprocessor("MooseApp", KokkosElementExtremeValue);

InputParameters
KokkosElementExtremeValue::validParams()
{
  InputParameters params =
      KokkosExtremeValueBase<KokkosElementVariablePostprocessor>::validParams();
  params.addCoupledVar("proxy_variable",
                       "The name of the variable to use to identify the location at which "
                       "the variable value should be taken; if not provided, this defaults "
                       "to the 'variable'.");
  params.addClassDescription(
      "Finds either the min or max elemental value of a variable over the domain.");
  return params;
}

KokkosElementExtremeValue::KokkosElementExtremeValue(const InputParameters & parameters)
  : KokkosExtremeValueBase<KokkosElementVariablePostprocessor>(parameters),
    _proxy_variable(isParamValid("proxy_variable") ? kokkosCoupledValue("proxy_variable") : _u)
{
}
