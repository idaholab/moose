//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GreaterThanLessThanPostprocessor.h"

// MOOSE includes
#include "MooseVariable.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "MooseMesh.h"
#include "AllLocalDofIndicesThread.h"

#include "libmesh/mesh_base.h"

registerMooseObject("MooseApp", GreaterThanLessThanPostprocessor);

InputParameters
GreaterThanLessThanPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Count number of DOFs of a non-linear variable that are greater than "
                             "or less than a given threshold");
  params.addRequiredParam<VariableName>("variable",
                                        "The name of the variable to conduct a comparison for");
  params.addParam<SubdomainName>("subdomain", "The subdomain that the variable lives on");
  params.addParam<Real>("value", 0, "The value to compare against");
  MooseEnum comparator("greater less", "greater");
  params.addParam<MooseEnum>(
      "comparator",
      comparator,
      "The comparison to perform between the variable and the provided value");
  return params;
}

GreaterThanLessThanPostprocessor::GreaterThanLessThanPostprocessor(
    const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _var(_fe_problem.getVariable(_tid,
                                 getParam<VariableName>("variable"),
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD)),
    _mesh(_fe_problem.mesh().getMesh()),
    _subdomain_restricted(isParamValid("subdomain")),
    _subdomain_id(_subdomain_restricted
                      ? _fe_problem.mesh().getSubdomainID(getParam<SubdomainName>("subdomain"))
                      : Moose::INVALID_BLOCK_ID),
    _value(getParam<Real>("value")),
    _comparator(getParam<MooseEnum>("comparator"))
{
}

void
GreaterThanLessThanPostprocessor::initialize()
{
  _count = 0;
}

void
GreaterThanLessThanPostprocessor::execute()
{
  AllLocalDofIndicesThread aldit(_fe_problem, {_var.name()});

  if (_subdomain_restricted)
  {
    ConstElemRange range(_mesh.active_local_subdomain_elements_begin(_subdomain_id),
                         _mesh.active_local_subdomain_elements_end(_subdomain_id));

    Threads::parallel_reduce(range, aldit);
  }
  else
  {
    ConstElemRange range(_mesh.active_local_elements_begin(), _mesh.active_local_elements_end());

    Threads::parallel_reduce(range, aldit);
  }

  auto && solution = _fe_problem.getNonlinearSystemBase().solution();

  if (_comparator == "greater")
  {
    for (auto dof : aldit.getDofIndices())
      if (solution(dof) > _value)
        ++_count;
  }
  else if (_comparator == "less")
  {
    for (auto dof : aldit.getDofIndices())
      if (solution(dof) < _value)
        ++_count;
  }
  else
    mooseError("Invalid comparator ", _comparator);
}

PostprocessorValue
GreaterThanLessThanPostprocessor::getValue()
{
  return _count;
}

void
GreaterThanLessThanPostprocessor::finalize()
{
  gatherSum(_count);
}
