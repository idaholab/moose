//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarVariable.h"

// MOOSE includes
#include "MooseVariableScalar.h"
#include "SubProblem.h"

#include "libmesh/dof_map.h"

template <>
InputParameters
validParams<ScalarVariable>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<VariableName>("variable", "Name of the variable");
  params.addParam<unsigned int>("component", 0, "Component to output for this variable");
  return params;
}

ScalarVariable::ScalarVariable(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _var(_subproblem.getScalarVariable(_tid, getParam<VariableName>("variable"))),
    _idx(getParam<unsigned int>("component"))
{
}

void
ScalarVariable::initialize()
{
}

void
ScalarVariable::execute()
{
}

Real
ScalarVariable::getValue()
{
  _var.reinit();

  Real returnval = std::numeric_limits<Real>::max();
  const DofMap & dof_map = _var.dofMap();
  const dof_id_type dof = _var.dofIndices()[_idx];
  if (dof >= dof_map.first_dof() && dof < dof_map.end_dof())
    returnval = _var.sln()[_idx];

  gatherMin(returnval);

  return returnval;
}
