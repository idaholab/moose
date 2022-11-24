//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateExecutionerAction.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "MooseApp.h"
#include "Executioner.h"
#include "Eigenvalue.h"
#include "FEProblem.h"
#include "EigenProblem.h"

registerMooseAction("MooseApp", CreateExecutionerAction, "setup_executioner");

InputParameters
CreateExecutionerAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add an Executioner object to the simulation.");
  params.addParam<bool>(
      "auto_preconditioning",
      true,
      "When true and a [Preconditioning] block does not exist, the application will attempt to use "
      "the correct preconditioning given the Executioner settings.");
  return params;
}

CreateExecutionerAction::CreateExecutionerAction(const InputParameters & params)
  : MooseObjectAction(params), _auto_preconditioning(getParam<bool>("auto_preconditioning"))
{
}

void
CreateExecutionerAction::act()
{
  std::shared_ptr<EigenProblem> eigen_problem = std::dynamic_pointer_cast<EigenProblem>(_problem);
  if (eigen_problem)
    _moose_object_pars.set<EigenProblem *>("_eigen_problem") = eigen_problem.get();
  _moose_object_pars.set<SubProblem *>("_subproblem") = static_cast<SubProblem *>(_problem.get());

  std::shared_ptr<Executioner> executioner =
      _factory.create<Executioner>(_type, "Executioner", _moose_object_pars);

  std::shared_ptr<Eigenvalue> eigen_executioner =
      std::dynamic_pointer_cast<Eigenvalue>(executioner);

  if ((eigen_problem == nullptr) != (eigen_executioner == nullptr))
    mooseError("Executioner is not consistent with each other; EigenExecutioner needs an "
               "EigenProblem, and Steady and Transient need a FEProblem");

  // If enabled, automatically create a Preconditioner if the [Preconditioning] block is not found
  if (_auto_preconditioning && !_awh.hasActions("add_preconditioning") &&
      _moose_object_pars.isParamValid("solve_type"))
    setupAutoPreconditioning();

  _app.setExecutioner(std::move(executioner));
}

void
CreateExecutionerAction::setupAutoPreconditioning()
{
  // If using NEWTON or LINEAR then automatically create SingleMatrixPreconditioner object with
  // full=true
  const MooseEnum & solve_type = _moose_object_pars.get<MooseEnum>("solve_type");
  if (((solve_type.find("NEWTON") != solve_type.items().end()) && (solve_type == "NEWTON")) ||
      ((solve_type.find("LINEAR") != solve_type.items().end()) && (solve_type == "LINEAR")))
  {
    // Action Parameters
    InputParameters params = _action_factory.getValidParams("SetupPreconditionerAction");
    params.set<std::string>("type") = "SMP";

    // Create the Action that will build the Preconditioner object
    std::shared_ptr<Action> ptr =
        _action_factory.create("SetupPreconditionerAction", "_moose_auto", params);

    // Set 'full=true'
    std::shared_ptr<MooseObjectAction> moa_ptr = std::static_pointer_cast<MooseObjectAction>(ptr);
    InputParameters & mo_params = moa_ptr->getObjectParams();
    mo_params.set<bool>("full") = true;

    _awh.addActionBlock(ptr);
  }
}
