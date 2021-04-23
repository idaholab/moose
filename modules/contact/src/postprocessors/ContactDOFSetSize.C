//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactDOFSetSize.h"

// MOOSE includes
#include "MooseVariable.h"
#include "FEProblemBase.h"
#include "NonlinearSystemBase.h"
#include "MooseMesh.h"
#include "AllLocalDofIndicesThread.h"

#include "libmesh/mesh_base.h"

registerMooseObject("ContactApp", ContactDOFSetSize);

InputParameters
ContactDOFSetSize::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<VariableName>("variable", "The name of the variable to test for contact");
  params.addRequiredParam<SubdomainName>("subdomain", "The subdomain that the variable lives on");
  params.addParam<Real>(
      "tolerance", TOLERANCE, "The tolerance for accepting that the variable indicates contact");
  params.addClassDescription("Outputs the number of dofs greater than a tolerance threshold "
                             "indicating mechanical contact");
  return params;
}

ContactDOFSetSize::ContactDOFSetSize(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _var(_fe_problem.getVariable(_tid,
                                 getParam<VariableName>("variable"),
                                 Moose::VarKindType::VAR_NONLINEAR,
                                 Moose::VarFieldType::VAR_FIELD_STANDARD)),
    _mesh(_fe_problem.mesh().getMesh()),
    _subdomain_id(_fe_problem.mesh().getSubdomainID(getParam<SubdomainName>("subdomain"))),
    _tolerance(getParam<Real>("tolerance"))
{
}

void
ContactDOFSetSize::initialize()
{
  _count = 0;
}

void
ContactDOFSetSize::execute()
{
  AllLocalDofIndicesThread aldit(_fe_problem, {_var.name()});

  // Get the element iterators corresponding to the subdomain id
  auto elem_begin = _mesh.active_local_subdomain_elements_begin(_subdomain_id);
  auto elem_end = _mesh.active_local_subdomain_elements_end(_subdomain_id);

  ConstElemRange range(elem_begin, elem_end);

  Threads::parallel_reduce(range, aldit);

  auto && solution = _fe_problem.getNonlinearSystemBase().solution();

  for (auto dof : aldit.getDofIndices())
    if (solution(dof) > _tolerance)
      ++_count;

  gatherSum(_count);
  _console << std::endl << "The number of nodes in contact is " << _count << std::endl << std::endl;
}

PostprocessorValue
ContactDOFSetSize::getValue()
{
  return _count;
}
