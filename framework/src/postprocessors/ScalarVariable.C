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

registerMooseObject("MooseApp", ScalarVariable);

InputParameters
ScalarVariable::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Returns the value of a scalar variable as a postprocessor value.");
  params.addRequiredParam<VariableName>("variable", "Name of the variable");
  params.addParam<unsigned int>("component", 0, "Component to output for this variable");
  return params;
}

ScalarVariable::ScalarVariable(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _var(_subproblem.getScalarVariable(_tid, getParam<VariableName>("variable"))),
    _idx(getParam<unsigned int>("component")),
    _value(0)
{
}

void
ScalarVariable::initialize()
{
}

void
ScalarVariable::execute()
{
  _var.reinit();
  _value = std::numeric_limits<Real>::max();
  const DofMap & dof_map = _var.dofMap();
  const dof_id_type dof = _var.dofIndices()[_idx];
  if (dof >= dof_map.first_dof() && dof < dof_map.end_dof())
    _value = _var.sln()[_idx];
}

Real
ScalarVariable::getValue()
{
  return _value;
}

void
ScalarVariable::finalize()
{
  gatherMin(_value);
}
