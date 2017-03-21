/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "VariableResidual.h"

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
