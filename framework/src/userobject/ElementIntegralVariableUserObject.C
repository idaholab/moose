//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegralVariableUserObject.h"

registerMooseObject("MooseApp", ElementIntegralVariableUserObject);

InputParameters
ElementIntegralVariableUserObject::validParams()
{
  InputParameters params = ElementIntegralUserObject::validParams();
  params.addClassDescription("computes a volume integral of a variable.");
  params.addRequiredCoupledVar("variable", "The name of the variable that this object operates on");
  return params;
}

ElementIntegralVariableUserObject::ElementIntegralVariableUserObject(
    const InputParameters & parameters)
  : ElementIntegralUserObject(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable"))
{
  addMooseVariableDependency(&mooseVariableField());
}

Real
ElementIntegralVariableUserObject::computeQpIntegral()
{
  return _u[_qp];
}
