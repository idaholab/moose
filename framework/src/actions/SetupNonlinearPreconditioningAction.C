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
#include "PetscSupport.h"

registerMooseAction("MooseApp", SetupNonlinearPreconditioningAction, "add_nl_preconditioning");

InputParameters
SetupNonlinearPreconditioningAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription(
      "Sets up nonlinear preconditioning for multi-system solves. The sub-block name becomes the "
      "PETSc options prefix for the outer SNES.");
  return params;
}

SetupNonlinearPreconditioningAction::SetupNonlinearPreconditioningAction(
    const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
SetupNonlinearPreconditioningAction::act()
{
  if (!_problem)
    return;

  auto nlp = _factory.create<NonlinearPreconditioning>(_type, _name, _moose_object_pars);

  _problem->setNonlinearPreconditioning(nlp);
}
