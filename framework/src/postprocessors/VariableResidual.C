//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableResidual.h"

// MOOSE includes
#include "MooseVariable.h"
#include "NonlinearSystemBase.h"

template <>
InputParameters
validParams<VariableResidual>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<VariableName>("variable",
                                        "The name of the variable to compute the residual for");
  return params;
}

VariableResidual::VariableResidual(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _var(_fe_problem.getVariable(_tid, getParam<VariableName>("variable")))
{
  if (_var.kind() != Moose::VAR_NONLINEAR)
    mooseError(name(), ": Residual can be computed only for nonlinear variables.");
}

void
VariableResidual::initialize()
{
  _var_residual = 0;
}

void
VariableResidual::execute()
{
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase();
  _var_residual = nl.system().calculate_norm(nl.RHS(), _var.number(), DISCRETE_L2);
}

PostprocessorValue
VariableResidual::getValue()
{
  return _var_residual;
}
