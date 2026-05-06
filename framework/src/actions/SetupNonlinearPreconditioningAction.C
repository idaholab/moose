//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupNonlinearPreconditioningAction.h"
#include "NonlinearPreconditioning.h"
#include "FEProblem.h"
#include "Factory.h"

registerMooseAction("MooseApp", SetupNonlinearPreconditioningAction, "add_nl_preconditioning");

InputParameters
SetupNonlinearPreconditioningAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Sets up nonlinear preconditioning for multi-system solves.");
  params.addRequiredParam<std::vector<NonlinearSystemName>>(
      "inner_nl_sys_names",
      "Names of nonlinear systems to solve as the inner nonlinear preconditioner before each "
      "outer Newton step.");
  return params;
}

SetupNonlinearPreconditioningAction::SetupNonlinearPreconditioningAction(
    const InputParameters & params)
  : Action(params)
{
}

void
SetupNonlinearPreconditioningAction::act()
{
  if (!_problem)
    return;

  auto obj_params = _factory.getValidParams("NonlinearPreconditioning");
  obj_params.set<std::vector<NonlinearSystemName>>("inner_nl_sys_names") =
      getParam<std::vector<NonlinearSystemName>>("inner_nl_sys_names");

  auto nlp = _factory.create<NonlinearPreconditioning>(
      "NonlinearPreconditioning", "NonlinearPreconditioning", obj_params);

  _problem->setNonlinearPreconditioning(nlp);
}
