//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosSideVariablePostprocessor.h"

#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

InputParameters
KokkosSideVariablePostprocessor::validParams()
{
  InputParameters params = SidePostprocessor::validParams();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable that this postprocessor operates on");
  return params;
}

KokkosSideVariablePostprocessor::KokkosSideVariablePostprocessor(const InputParameters & parameters)
  : SidePostprocessor(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(kokkosCoupledValue("variable")),
    _grad_u(kokkosCoupledGradient("variable"))
{
  addMooseVariableDependency(&mooseVariableField());
}
