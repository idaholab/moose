//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideVariablePostprocessor.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"

#include "libmesh/quadrature.h"

InputParameters
SideVariablePostprocessor::validParams()
{
  InputParameters params = SidePostprocessor::validParams();
  params.addRequiredCoupledVar("variable",
                               "The name of the variable that this postprocessor operates on");
  return params;
}

SideVariablePostprocessor::SideVariablePostprocessor(const InputParameters & parameters)
  : SidePostprocessor(parameters),
    MooseVariableInterface<Real>(this,
                                 false,
                                 "variable",
                                 Moose::VarKindType::VAR_ANY,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD),
    _u(coupledValue("variable")),
    _grad_u(coupledGradient("variable")),
    _qp(0)
{
  addMooseVariableDependency(&mooseVariableField());
}

void
SideVariablePostprocessor::execute()
{
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    computeQpValue();
}
