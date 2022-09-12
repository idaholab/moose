//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMCreateMeshAction.h"
#include "THMMesh.h"
#include "FEProblemBase.h"
#include "CreateProblemAction.h"
#include "Factory.h"
#include "MooseApp.h"
#include "ActionWarehouse.h"

registerMooseAction("ThermalHydraulicsApp", THMCreateMeshAction, "setup_mesh");
registerMooseAction("ThermalHydraulicsApp", THMCreateMeshAction, "uniform_refine_mesh");

InputParameters
THMCreateMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Action that creates an empty mesh (in case one was not already "
                             "created) and also builds THMProblem.");
  return params;
}

THMCreateMeshAction::THMCreateMeshAction(const InputParameters & params) : Action(params) {}

void
THMCreateMeshAction::act()
{
  if (_current_task == "setup_mesh")
  {
    if (!_mesh)
    {
      const std::string class_name = "THMMesh";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MooseEnum>("dim") = "3";
      params.set<unsigned int>("patch_size") = 1;
      // We need to go through the Factory to create the THMMesh so
      // that its params object is properly added to the Warehouse.
      _mesh = _factory.create<THMMesh>(class_name, "THM:mesh", params);
    }

    if (!_mesh->hasMeshBase())
      _mesh->setMeshBase(_mesh->buildMeshBaseObject());

    if (!_problem)
    {
      const std::string class_name = "THMProblem";
      InputParameters params = _factory.getValidParams(class_name);
      _app.parser().extractParams("", params); // extract global params
      // apply common parameters of the object held by CreateProblemAction to honor user inputs in
      // [Problem]
      auto p = _awh.getActionByTask<CreateProblemAction>("create_problem");
      if (p)
        params.applyParameters(p->getObjectParams());
      params.set<MooseMesh *>("mesh") = _mesh.get();
      params.set<bool>("use_nonlinear") = _app.useNonlinear();
      _problem = _factory.create<FEProblemBase>(class_name, "THM:problem", params);
    }
  }
  else if (_current_task == "uniform_refine_mesh")
  {
    auto level = _app.getParam<unsigned int>("refinements");
    _mesh->setUniformRefineLevel(level, false);
  }
}
