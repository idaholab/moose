//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubChannelAddVariablesAction.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "AddAuxVariableAction.h"
#include "SubChannelApp.h"
#include <string_view>
#include "AddMeshGeneratorAction.h"

registerMooseAction("SubChannelApp", SubChannelAddVariablesAction, "meta_action");

InputParameters
SubChannelAddVariablesAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Adds the variables associated with the subchannel problem");
  params.addParam<bool>(
      "deformation",
      false,
      "Boolean that activates the deformation effect based on values for: displacement, Dpin");
  params.addParam<bool>("full_output", false, "Add optional subchannel output variables");
  return params;
}

SubChannelAddVariablesAction::SubChannelAddVariablesAction(const InputParameters & parameters)
  : Action(parameters),
    _fe_family(AddVariableAction::getNonlinearVariableFamilies()),
    _fe_order(AddVariableAction::getNonlinearVariableOrders())
{
}

void
SubChannelAddVariablesAction::act()
{
  bool pin_mesh_exist = false;
  bool duct_mesh_exist = false;

  const auto & mesh_actions = _awh.getActions<AddMeshGeneratorAction>();

  for (const auto * mesh_action : mesh_actions)
  {
    if (!mesh_action->parameters().isParamValid("type"))
      continue;

    const auto & generator_type = mesh_action->getParam<std::string>("type");

    if (generator_type == "SCMTriDuctMeshGenerator" || generator_type == "SCMQuadDuctMeshGenerator")
      duct_mesh_exist = true;

    if (generator_type == "SCMTriPinMeshGenerator" || generator_type == "SCMQuadPinMeshGenerator")
      pin_mesh_exist = true;
  }

  std::vector<std::string> var_names = {SubChannelApp::MASS_FLOW_RATE,
                                        SubChannelApp::SURFACE_AREA,
                                        SubChannelApp::SUM_CROSSFLOW,
                                        SubChannelApp::PRESSURE,
                                        SubChannelApp::WETTED_PERIMETER,
                                        SubChannelApp::LINEAR_HEAT_RATE,
                                        SubChannelApp::ENTHALPY,
                                        SubChannelApp::TEMPERATURE,
                                        SubChannelApp::DENSITY,
                                        SubChannelApp::VISCOSITY};

  if (pin_mesh_exist)
  {
    var_names.push_back(SubChannelApp::PIN_TEMPERATURE);
    var_names.push_back(SubChannelApp::PIN_DIAMETER);
    var_names.push_back(SubChannelApp::HEAT_TRANSFER_COEFFICIENT);
  }

  if (duct_mesh_exist)
  {
    var_names.push_back(SubChannelApp::DUCT_HEAT_FLUX);
    var_names.push_back(SubChannelApp::DUCT_TEMPERATURE);
  }

  const bool full_output = getParam<bool>("full_output");

  if (full_output)
  {
    var_names.push_back(SubChannelApp::PRESSURE_DROP);
    var_names.push_back(SubChannelApp::FRICTION_FACTOR);
  }

  const bool deformation = getParam<bool>("deformation");

  if (deformation)
  {
    var_names.push_back(SubChannelApp::DISPLACEMENT);
  }

  // Get a list of the already existing AddAuxVariableAction
  const auto & aux_actions = _awh.getActions<AddAuxVariableAction>();

  for (auto & vn : var_names)
  {
    const std::string class_name = "AddAuxVariableAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<MooseEnum>("family") = _fe_family;
    params.set<MooseEnum>("order") = _fe_order;

    std::shared_ptr<Action> action =
        std::static_pointer_cast<Action>(_action_factory.create(class_name, vn, params));

    //  Avoid trying (and failing) to override the user variable selection
    bool add_action = true;
    for (const auto aux_action : aux_actions)
      if (aux_action->name() == vn)
        add_action = false;

    if (add_action)
      _awh.addActionBlock(action);
  }
}
