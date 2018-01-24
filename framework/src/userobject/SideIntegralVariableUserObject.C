//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralVariableUserObject.h"

template <>
InputParameters
validParams<SideIntegralVariableUserObject>()
{
  InputParameters params = validParams<SideIntegralUserObject>();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable that this boundary condition applies to");
  return params;
}

SideIntegralVariableUserObject::SideIntegralVariableUserObject(const InputParameters & parameters)
  : SideIntegralUserObject(parameters),
    MooseVariableInterface(this, false),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable"))
{
  addMooseVariableDependency(mooseVariable());
}

Real
SideIntegralVariableUserObject::computeQpIntegral()
{
  return _u[_qp];
}
