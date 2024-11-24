//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseApp.h"
#include "CSGOnlyAction.h"
#include "CSGBase.h"

registerMooseAction("MooseApp", CSGOnlyAction, "csg_only");
registerMooseAction("MooseApp", CSGOnlyAction, "setup_mesh");
registerMooseAction("MooseApp", CSGOnlyAction, "execute_csg_generators");

InputParameters
CSGOnlyAction::validParams()
{
  return Action::validParams();
}

CSGOnlyAction::CSGOnlyAction(const InputParameters & params) : Action(params) {}

void
CSGOnlyAction::act()
{
  if (_current_task == "setup_mesh")
  {
    if (!_mesh)
      mooseError("Cannot generate CSG mesh without a [Mesh] block in the input file");
    _app.getMeshGeneratorSystem().setCSGOnly();
  }
  else if (_current_task == "execute_csg_generators")
  {
    const auto ordered_mg = _app.getMeshGeneratorSystem().getOrderedMeshGenerators();
    for (const auto & generator_set : ordered_mg)
      for (const auto & generator : generator_set)
        _csg_mesh = generator->generateInternalCSG();
  }
  else if (_current_task == "csg_only")
  {
    Moose::out << "Outputting CSGBase object at this step\n";
    _csg_mesh->generateOutput();
  }
}
