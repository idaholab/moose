//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
    _proxy_variable(isParamValid("proxy_variable") ? coupledValue("proxy_variable") : _u),
    _proxy_var(isParamValid("proxy_variable") ? getVar("proxy_variable", 0) : nullptr)
{
}

void
NodalExtremeValue::execute()
{
  const bool have_dofs = _var->dofIndices().size();
  if (_proxy_var)
    mooseAssert(static_cast<bool>(_proxy_var->dofIndices().size()) == have_dofs,
                "Should not use variables that don't have coincident dof maps");
  if (have_dofs)
    computeExtremeValue();
}

std::pair<Real, Real>
NodalExtremeValue::getProxyValuePair()
{
  return std::make_pair(_proxy_variable[_qp], _u[_qp]);
}
