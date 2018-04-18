//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralVariablePostprocessor.h"

registerMooseObject("MooseApp", SideIntegralVariablePostprocessor);

template <>
InputParameters
validParams<SideIntegralVariablePostprocessor>()
{
  InputParameters params = validParams<SideIntegralPostprocessor>();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable that this boundary condition applies to");
  return params;
}

SideIntegralVariablePostprocessor::SideIntegralVariablePostprocessor(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable"))
{
  addMooseVariableDependency(mooseVariable());
}

Real
SideIntegralVariablePostprocessor::computeQpIntegral()
{
  return _u[_qp];
}
