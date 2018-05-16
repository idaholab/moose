//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateProblemAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "MooseApp.h"

registerMooseAction("MooseApp", CreateProblemAction, "create_problem");

template <>
InputParameters
validParams<CreateProblemAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  params.addParam<std::string>("type", "FEProblem", "Problem type");
  params.addParam<std::string>("name", "MOOSE Problem", "The name the problem");
  return params;
}

CreateProblemAction::CreateProblemAction(InputParameters parameters) : MooseObjectAction(parameters)
{
}

void
CreateProblemAction::act()
{
  // build the problem only if we have mesh
  if (_mesh.get() != NULL && _pars.isParamSetByUser("type"))
  {
    // when this action is built by parser with Problem input block, this action
    // must act i.e. create a problem. Thus if a problem has been created, it will error out.
    if (_problem)
      mooseError("Trying to build a problem but problem has already existed");

    _moose_object_pars.set<MooseMesh *>("mesh") = _mesh.get();
    _moose_object_pars.set<bool>("use_nonlinear") = _app.useNonlinear();

    _problem = _factory.createUnique<FEProblemBase>(
        _type, getParam<std::string>("name"), _moose_object_pars);
  }
}
