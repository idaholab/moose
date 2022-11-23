//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateTHMProblemAction.h"
#include "CreateProblemAction.h"
#include "FEProblemBase.h"

registerMooseAction("ThermalHydraulicsApp", CreateTHMProblemAction, "THM:create_thm_problem");

InputParameters
CreateTHMProblemAction::validParams()
{
  InputParameters params = Action::validParams();
  params += THMAppInterface::validParams();

  params.addClassDescription("Creates the THM problem.");

  return params;
}

CreateTHMProblemAction::CreateTHMProblemAction(const InputParameters & params)
  : Action(params), THMAppInterface(params)
{
}

void
CreateTHMProblemAction::act()
{
  if (!_problem)
  {
    auto & thm_app = getTHMApp();

    // create the THM problem
    const std::string class_name = "THMProblem";
    InputParameters params = _factory.getValidParams(class_name);
    _app.parser().extractParams("", params); // extract global params
    // apply common parameters of the object held by CreateProblemAction to honor user inputs in
    // [Problem]
    auto create_problem_action = _awh.getActionByTask<CreateProblemAction>("create_problem");
    if (create_problem_action)
      params.applyParameters(create_problem_action->getObjectParams());
    params.set<MooseMesh *>("mesh") = _mesh.get();
    params.set<bool>("use_nonlinear") = _app.useNonlinear();
    _problem = _factory.create<FEProblemBase>(class_name, "THM:problem", params);

    // set the THM problem in the THM app
    thm_app.setTHMProblem(*_problem);
  }
}
