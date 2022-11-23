//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateTHMMeshGeneratorAction.h"

registerMooseAction("ThermalHydraulicsApp", CreateTHMMeshGeneratorAction, "meta_action");

InputParameters
CreateTHMMeshGeneratorAction::validParams()
{
  InputParameters params = Action::validParams();

  params.addClassDescription("Creates the THM mesh generator.");

  return params;
}

CreateTHMMeshGeneratorAction::CreateTHMMeshGeneratorAction(const InputParameters & params)
  : Action(params)
{
}

void
CreateTHMMeshGeneratorAction::act()
{
  {
    const std::string action_class_name = "AddMeshGeneratorAction";
    const std::string obj_class_name = "THMMeshGenerator";

    InputParameters action_params = _action_factory.getValidParams(action_class_name);
    action_params.set<std::string>("type") = obj_class_name;

    std::shared_ptr<MooseObjectAction> action = std::static_pointer_cast<MooseObjectAction>(
        _action_factory.create(action_class_name, "thm_mesh_generator", action_params));

    _awh.addActionBlock(action);
  }
}
