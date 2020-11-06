//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralVariableUserObject.h"

defineLegacyParams(SideIntegralVariableUserObject);

InputParameters
SideIntegralVariableUserObject::validParams()
{
  InputParameters params = SideIntegralUserObject::validParams();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable that this boundary condition applies to");
  return params;
}

SideIntegralVariableUserObject::SideIntegralVariableUserObject(const InputParameters & parameters)
  : SideIntegralUserObject(parameters),
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
SideIntegralVariableUserObject::computeQpIntegral()
{
  return _u[_qp];
}
