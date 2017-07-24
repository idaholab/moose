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
