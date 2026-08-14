//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "SetupLineSearchAction.h"
#include "Factory.h"
#include "LineSearch.h"
#include "FEProblem.h"
#include "NonlinearSystemBase.h"
#include "PetscSupport.h"

registerMooseAction("MooseApp", SetupLineSearchAction, "add_line_search");

InputParameters
SetupLineSearchAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a LineSearch object to a nonlinear system.");
  return params;
}

SetupLineSearchAction::SetupLineSearchAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
SetupLineSearchAction::act()
{
  if (_problem.get() != NULL)
  {
    std::shared_ptr<LineSearch> line_search =
        _factory.create<LineSearch>(_type, _name, _moose_object_pars);

    const auto nl_sys_num = line_search->_nl_sys_num;
    line_search->_nl.setLineSearch(line_search);
    Moose::PetscSupport::attachLineSearchObject(*_problem, nl_sys_num);
  }
}
