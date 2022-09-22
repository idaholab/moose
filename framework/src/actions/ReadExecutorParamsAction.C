//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReadExecutorParamsAction.h"
#include "Factory.h"
#include "PetscSupport.h"
#include "MooseApp.h"
#include "Executor.h"
#include "Eigenvalue.h"
#include "FEProblem.h"
#include "EigenProblem.h"

registerMooseAction("MooseApp", ReadExecutorParamsAction, "read_executor");

InputParameters
ReadExecutorParamsAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add an Executor object to the simulation.");
  params.addParam<bool>(
      "auto_preconditioning",
      true,
      "When true and a [Preconditioning] block does not exist, the application will attempt to use "
      "the correct preconditioning given the Executor settings.");
  return params;
}

ReadExecutorParamsAction::ReadExecutorParamsAction(const InputParameters & params)
  : MooseObjectAction(params), _auto_preconditioning(getParam<bool>("auto_preconditioning"))
{
}

void
ReadExecutorParamsAction::act()
{
  // If enabled, automatically create a Preconditioner if the [Preconditioning] block is not found
  if (_auto_preconditioning && !_awh.hasActions("add_preconditioning") &&
      _moose_object_pars.isParamValid("solve_type"))
    setupAutoPreconditioning();

  _app.addExecutorParams(_type, _name, _moose_object_pars);
}

void
ReadExecutorParamsAction::setupAutoPreconditioning()
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
