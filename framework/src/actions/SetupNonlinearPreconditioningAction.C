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
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Sets up nonlinear preconditioning for multi-system solves. The sub-block name becomes the "
      "PETSc options prefix for the outer SNES.");
  params.addRequiredParam<std::vector<NonlinearSystemName>>(
      "inner_nl_sys_names",
      "Names of nonlinear systems to solve as the inner nonlinear preconditioner before each "
      "outer Newton step.");
  params.addParam<MultiMooseEnum>(
      "petsc_options", Moose::PetscSupport::getCommonPetscFlags(), "Singleton PETSc options");
  params.addParam<MultiMooseEnum>("petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs");
  params.addParam<std::vector<std::string>>(
      "petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\")");
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

  // Store PETSc options under the sub-block-name prefix so the outer SNES can pick them up
  // via SNESSetOptionsPrefix + SNESSetFromOptions in wireToSNES().
  Moose::PetscSupport::storePetscOptions(*_problem, _name + "_", *this);

  auto obj_params = _factory.getValidParams("NonlinearPreconditioning");
  obj_params.set<std::vector<NonlinearSystemName>>("inner_nl_sys_names") =
      getParam<std::vector<NonlinearSystemName>>("inner_nl_sys_names");

  auto nlp = _factory.create<NonlinearPreconditioning>("NonlinearPreconditioning", _name, obj_params);

  _problem->setNonlinearPreconditioning(nlp);
}
