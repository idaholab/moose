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

  std::vector<SubdomainName> fluid_blocks;
  std::vector<SubdomainName> pin_blocks;
  std::vector<SubdomainName> duct_blocks;

  const auto & mesh_actions = _awh.getActions<AddMeshGeneratorAction>();

  for (const auto * mesh_action : mesh_actions)
  {
    if (!mesh_action->parameters().isParamValid("type"))
      continue;

    const auto & generator_type = mesh_action->getParam<std::string>("type");

    if (generator_type == "SCMQuadSubChannelMeshGenerator" ||
        generator_type == "SCMTriSubChannelMeshGenerator")
    {
      // Use the user-provided mesh generator block name
      fluid_blocks = {mesh_action->name()};
    }

    if (generator_type == "SCMTriPinMeshGenerator" || generator_type == "SCMQuadPinMeshGenerator")
    {
      pin_mesh_exist = true;
      // Use the user-provided mesh generator block name
      pin_blocks = {mesh_action->name()};
    }

    if (generator_type == "SCMTriDuctMeshGenerator" || generator_type == "SCMQuadDuctMeshGenerator")
    {
      duct_mesh_exist = true;
      // Use the user-provided mesh generator block name
      duct_blocks = {mesh_action->name()};
    }
  }

  // Backward-compatible fallback if no explicit subchannel mesh generator name was found
  if (fluid_blocks.empty())
    fluid_blocks = {"sub_channel"};

  std::vector<std::pair<std::string, std::vector<SubdomainName>>> vars_to_add;

  // Always-fluid variables
  vars_to_add.push_back({SubChannelApp::MASS_FLOW_RATE, fluid_blocks});
  vars_to_add.push_back({SubChannelApp::SURFACE_AREA, fluid_blocks});
  vars_to_add.push_back({SubChannelApp::SUM_CROSSFLOW, fluid_blocks});
  vars_to_add.push_back({SubChannelApp::PRESSURE, fluid_blocks});
  vars_to_add.push_back({SubChannelApp::WETTED_PERIMETER, fluid_blocks});
  vars_to_add.push_back({SubChannelApp::ENTHALPY, fluid_blocks});
  vars_to_add.push_back({SubChannelApp::TEMPERATURE, fluid_blocks});
  vars_to_add.push_back({SubChannelApp::DENSITY, fluid_blocks});
  vars_to_add.push_back({SubChannelApp::VISCOSITY, fluid_blocks});
  vars_to_add.push_back({SubChannelApp::DISPLACEMENT, fluid_blocks});

  // q_prime lives on pins if pins exist, otherwise on the fluid mesh
  if (pin_mesh_exist)
    vars_to_add.push_back({SubChannelApp::LINEAR_HEAT_RATE, pin_blocks});
  else
    vars_to_add.push_back({SubChannelApp::LINEAR_HEAT_RATE, fluid_blocks});

  // Pin-only variables
  if (pin_mesh_exist)
  {
    vars_to_add.push_back({SubChannelApp::PIN_TEMPERATURE, pin_blocks});
    vars_to_add.push_back({SubChannelApp::PIN_DIAMETER, pin_blocks});
    vars_to_add.push_back({SubChannelApp::HEAT_TRANSFER_COEFFICIENT, pin_blocks});
  }

  // Duct-only variables
  if (duct_mesh_exist)
  {
    vars_to_add.push_back({SubChannelApp::DUCT_HEAT_FLUX, duct_blocks});
    vars_to_add.push_back({SubChannelApp::DUCT_TEMPERATURE, duct_blocks});
  }

  if (getParam<bool>("full_output"))
  {
    vars_to_add.push_back({SubChannelApp::PRESSURE_DROP, fluid_blocks});
    vars_to_add.push_back({SubChannelApp::FRICTION_FACTOR, fluid_blocks});
  }

  const auto & aux_actions = _awh.getActions<AddAuxVariableAction>();

  for (const auto & var_info : vars_to_add)
  {
    const std::string & vn = var_info.first;
    const std::vector<SubdomainName> & blocks = var_info.second;

    bool add_action = true;
    for (const auto * aux_action : aux_actions)
    {
      if (aux_action->name() == vn)
      {
        add_action = false;
        break;
      }
    }

    if (!add_action)
      continue;

    const std::string class_name = "AddAuxVariableAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<MooseEnum>("family") = _fe_family;
    params.set<MooseEnum>("order") = _fe_order;
    params.set<std::vector<SubdomainName>>("block") = blocks;

    std::shared_ptr<Action> action =
        std::static_pointer_cast<Action>(_action_factory.create(class_name, vn, params));

    _awh.addActionBlock(action);
  }
}
