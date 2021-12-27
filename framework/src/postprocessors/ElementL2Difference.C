//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementL2Difference.h"
#include "Function.h"

registerMooseObject("MooseApp", ElementL2Difference);

defineLegacyParams(ElementL2Difference);

InputParameters
ElementL2Difference::validParams()
{
  InputParameters params = ElementIntegralVariablePostprocessor::validParams();
  params.addRequiredCoupledVar("other_variable", "The variable to compare to");

  params.addClassDescription("Computes the element-wise L2 difference between the current variable "
                             "and a coupled variable.");
  return params;
}

ElementL2Difference::ElementL2Difference(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters),
    _other_var(coupledValue("other_variable")),
    _variables_match(getVar("other_variable", 0)->name() == getVar("variable", 0)->name())
{
  if (_variables_match)
  {
    if (!_fe_problem.isTransient())
      paramError("other_variable",
                 "When using a 'Steady' executioner, cannot compare against "
                 "previous values; 'other_variable' cannot be the same as 'variable'");
    else
      _u_old = &coupledValueOld("variable");
  }
}

Real
ElementL2Difference::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
ElementL2Difference::computeQpIntegral()
{
  Real other = _variables_match ? (*_u_old)[_qp] : _other_var[_qp];
  Real diff = _u[_qp] - other;
  return diff * diff;
}
