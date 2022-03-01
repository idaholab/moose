//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalVariablePostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

InputParameters
NodalVariablePostprocessor::validParams()
{
  InputParameters params = NodalPostprocessor::validParams();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable that this postprocessor operates on");
  return params;
}

NodalVariablePostprocessor::NodalVariablePostprocessor(const InputParameters & parameters)
  : NodalPostprocessor(parameters),
    MooseVariableInterface<Real>(this,
                                 true,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(coupledValue("variable"))
{
}
