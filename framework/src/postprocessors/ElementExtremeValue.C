//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementExtremeValue.h"

registerMooseObject("MooseApp", ElementExtremeValue);

InputParameters
ElementExtremeValue::validParams()
{
  InputParameters params = ExtremeValueBase<ElementVariablePostprocessor>::validParams();
  params.addCoupledVar("proxy_variable",
                       "The name of the variable to use to identify the location at which "
                       "the variable value should be taken; if not provided, this defaults "
                       "to the 'variable'.");
  params.addClassDescription(
      "Finds either the min or max elemental value of a variable over the domain.");
  return params;
}

ElementExtremeValue::ElementExtremeValue(const InputParameters & parameters)
  : ExtremeValueBase<ElementVariablePostprocessor>(parameters),
    _proxy_variable(isParamValid("proxy_variable") ? coupledValue("proxy_variable") : _u)
{
  _use_proxy = isParamValid("proxy_variable");
}

std::pair<Real, Real>
ElementExtremeValue::getProxyValuePair()
{
  return std::make_pair(_proxy_variable[_qp], _u[_qp]);
}
