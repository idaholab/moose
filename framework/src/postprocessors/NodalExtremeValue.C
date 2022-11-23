//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalExtremeValue.h"

#include <algorithm>
#include <limits>

registerMooseObject("MooseApp", NodalExtremeValue);

InputParameters
NodalExtremeValue::validParams()
{
  InputParameters params = ExtremeValueBase<NodalVariablePostprocessor>::validParams();
  params.addCoupledVar("proxy_variable",
                       "The name of the variable to use to identify the location at which "
                       "the variable value should be taken; if not provided, this defaults "
                       "to the 'variable'.");
  params.addClassDescription(
      "Finds either the min or max elemental value of a variable over the domain.");
  return params;
}

NodalExtremeValue::NodalExtremeValue(const InputParameters & parameters)
  : ExtremeValueBase<NodalVariablePostprocessor>(parameters),
    _proxy_variable(isParamValid("proxy_variable") ? coupledValue("proxy_variable") : _u)
{
  _use_proxy = isParamValid("proxy_variable");
}

std::pair<Real, Real>
NodalExtremeValue::getProxyValuePair()
{
  return std::make_pair(_proxy_variable[_qp], _u[_qp]);
}
