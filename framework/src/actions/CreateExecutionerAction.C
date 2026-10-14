//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#include "FEProblem.h"
#include "EigenProblem.h"
#include "EigenProblemSolve.h"
#include "SetupPreconditionerAction.h"

registerMooseAction("MooseApp", CreateExecutionerAction, "setup_executioner");

InputParameters
CreateExecutionerAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add an Executioner object to the simulation.");
  params.addParam<bool>(
      "auto_preconditioning",
      true,
      "When true, the application will attempt to use the correct preconditioning given the "
      "Executioner settings for the nonlinear systems that do not have a preconditioner in the "
      "[Preconditioning] block.");
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
  _moose_object_pars.set<SubProblem *>("_subproblem") = cast_ptr<SubProblem *>(_problem.get());

  std::shared_ptr<Executioner> executioner =
      _factory.create<Executioner>(_type, "Executioner", _moose_object_pars);

  if ((eigen_problem != nullptr) != executioner->hasSolveObject<EigenProblemSolve>())
    mooseError("Executioner is not consistent with each other; EigenExecutioner needs an "
               "EigenProblem, and Steady and Transient need a FEProblem");

  // If enabled, automatically create a Preconditioner for the nonlinear systems that do not have
  // one in the [Preconditioning] block
  if (_auto_preconditioning && _moose_object_pars.isParamValid("solve_type"))
    setupAutoPreconditioning();

  _app.setExecutioner(std::move(executioner));
}

void
CreateExecutionerAction::setupAutoPreconditioning()
{
  // Nonlinear systems that already have a preconditioner from the [Preconditioning] block. Other
  // actions on the preconditioning task (e.g. Physics) are not tied to a nonlinear system, so
  // their presence disables the automatic preconditioner entirely.
  std::set<unsigned int> preconditioned_nl_sys;
  for (const auto action : _awh.getActionListByName("add_preconditioning"))
  {
    const auto pc_action = dynamic_cast<const SetupPreconditionerAction *>(action);
    if (!pc_action)
      return;
    const auto & pc_params = pc_action->getObjectParams();
    preconditioned_nl_sys.insert(
        pc_params.isParamValid("nl_sys")
            ? _problem->nlSysNum(pc_params.get<NonlinearSystemName>("nl_sys"))
            : 0);
  }

  // Without a [Preconditioning] block, only NEWTON and LINEAR solves need a preconditioner, which
  // then uses the full coupling. A user preconditioner sets a custom coupling matrix, so every
  // other nonlinear system then needs a preconditioner to set its coupling matrix, whatever the
  // solve type. Its coupling follows the effective solve type, which a user preconditioner can
  // override.
  if (preconditioned_nl_sys.empty())
  {
    // Compare as a string because the eigenvalue solve type enumeration has no LINEAR item
    const std::string & solve_type = _moose_object_pars.get<MooseEnum>("solve_type");
    if (solve_type != "NEWTON" && solve_type != "LINEAR")
      return;
  }

  for (const auto & nl_sys_name : _problem->getNonlinearSystemNames())
  {
    if (preconditioned_nl_sys.count(_problem->nlSysNum(nl_sys_name)))
      continue;

    // Action Parameters
    InputParameters params = _action_factory.getValidParams("SetupPreconditionerAction");
    params.set<std::string>("type") = "SMP";

    // Associate errors with "solve_type"
    associateWithParameter(_moose_object_pars, "solve_type", params);

    // Create the Action that will build the Preconditioner object
    std::shared_ptr<Action> ptr = _action_factory.create(
        "SetupPreconditionerAction",
        MooseUtils::join(
            std::vector<std::string>({"_moose_auto", static_cast<std::string>(nl_sys_name)}), "_"),
        params);

    // The SingleMatrixPreconditioner defaults to the full coupling for NEWTON and LINEAR solves
    std::shared_ptr<MooseObjectAction> moa_ptr = std::static_pointer_cast<MooseObjectAction>(ptr);
    InputParameters & mo_params = moa_ptr->getObjectParams();
    mo_params.set<NonlinearSystemName>("nl_sys") = nl_sys_name;

    _awh.addActionBlock(ptr);
  }
}
