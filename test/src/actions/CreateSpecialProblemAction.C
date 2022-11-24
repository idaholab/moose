//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateSpecialProblemAction.h"

#include "MooseApp.h"
#include "FEProblemBase.h"
#include "CreateProblemAction.h"

registerMooseAction("MooseTestApp", CreateSpecialProblemAction, "create_problem_custom");

InputParameters
CreateSpecialProblemAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<std::string>("name", "Test Problem", "The name of the Problem object");
  return params;
}

CreateSpecialProblemAction::CreateSpecialProblemAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
CreateSpecialProblemAction::act()
{
  if (_problem)
  {
    // problem has been created by CreateProblemAction
    if (_problem->type() != "MooseTestProblem")
      mooseError("TestProblem input block requires Problem/type to be MooseTestProblem");
  }
  else
  {
    if (_app.useEigenvalue())
      mooseError("MooseTestProblem does not work with Eigenvalue executioner");

    auto params = _factory.getValidParams("MooseTestProblem");

    // apply comman parameters of the object held by CreateProblemAction to honor user inputs in
    // [Problem]
    auto p = _awh.getActionByTask<CreateProblemAction>("create_problem");
    if (p)
      params.applyParameters(p->getObjectParams());

    params.set<MooseMesh *>("mesh") = _mesh.get();
    params.set<bool>("use_nonlinear") = _app.useNonlinear();

    _problem =
        _factory.create<FEProblemBase>("MooseTestProblem", getParam<std::string>("name"), params);
  }
}
