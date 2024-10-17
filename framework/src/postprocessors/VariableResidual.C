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

// libMesh includes
#include "libmesh/enum_norm_type.h"

registerMooseObject("MooseApp", VariableResidual);

InputParameters
VariableResidual::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable",
                                        "The name of the variable to compute the residual for");

  params.addClassDescription(
      "Computes the L2 norm of the residual of a single variable in the solution vector.");
  return params;
}

VariableResidual::VariableResidual(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _var(_fe_problem.getVariable(_tid,
                                 getParam<VariableName>("variable"),
                                 Moose::VarKindType::VAR_SOLVER,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD))
{
}

void
VariableResidual::initialize()
{
  _var_residual = 0;
}

void
VariableResidual::execute()
{
  NonlinearSystemBase & nl = _fe_problem.getNonlinearSystemBase(_sys.number());
  _var_residual = nl.system().calculate_norm(nl.RHS(), _var.number(), libMesh::DISCRETE_L2);
}

PostprocessorValue
VariableResidual::getValue() const
{
  return _var_residual;
}
