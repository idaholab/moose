//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegralVariablePostprocessor.h"

registerMooseObject("MooseApp", ElementIntegralVariablePostprocessor);

InputParameters
ElementIntegralVariablePostprocessor::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("variable", "The name of the variable that this object operates on");
  params.addClassDescription("Computes a volume integral of the specified variable");
  params.addParam<bool>(
      "use_absolute_value", false, "Whether to use abolsute value of the variable or not");
  return params;
}

ElementIntegralVariablePostprocessor::ElementIntegralVariablePostprocessor(
    const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable")),
    _use_abs_value(getParam<bool>("use_absolute_value"))
{
  addMooseVariableDependency(&mooseVariableField());
}

Real
ElementIntegralVariablePostprocessor::computeQpIntegral()
{
  if (_use_abs_value)
    return std::abs(_u[_qp]);
  else
    return _u[_qp];
}
