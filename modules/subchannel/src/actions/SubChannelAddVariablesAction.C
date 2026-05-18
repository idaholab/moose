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
#include "FEProblemBase.h"
#include "SubChannelApp.h"
#include "AddMeshGeneratorAction.h"
#include "AddICAction.h"
#include "AddInitialConditionAction.h"
#include "MooseUtils.h"
#include "QuadSubChannelMesh.h"
#include "SubChannelMesh.h"
#include "TriSubChannelMesh.h"

registerMooseAction("SubChannelApp", SubChannelAddVariablesAction, "add_aux_variable");
registerMooseAction("SubChannelApp", SubChannelAddVariablesAction, "add_ic");

InputParameters
SubChannelAddVariablesAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Adds the variables associated with the subchannel problem");
  params.addParam<bool>("full_output", false, "Add optional subchannel output variables");
  return params;
}

SubChannelAddVariablesAction::SubChannelAddVariablesAction(const InputParameters & parameters)
  : Action(parameters),
    _fe_family(AddAuxVariableAction::getAuxVariableFamilies()),
    _fe_order(AddAuxVariableAction::getAuxVariableOrders())
{
}

void
SubChannelAddVariablesAction::addAuxVariable(const std::string & var_name,
                                             const std::vector<SubdomainName> & blocks)
{
  const auto & aux_actions = _awh.getActions<AddAuxVariableAction>();

  for (const auto * aux_action : aux_actions)
    if (aux_action->name() == var_name)
      return;

  auto params = _factory.getValidParams("MooseVariable");
  params.set<MooseEnum>("family") = _fe_family;
  params.set<MooseEnum>("order") = _fe_order;
  params.set<std::vector<SubdomainName>>("block") = blocks;

  _problem->addAuxVariable("MooseVariable", var_name, params);
}

void
SubChannelAddVariablesAction::act()
{
  if (_current_task == "add_ic")
  {
    addInitialConditions();
    return;
  }

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
    // HTC is a subchannel-average output stored on the fluid mesh.
    vars_to_add.push_back({SubChannelApp::HEAT_TRANSFER_COEFFICIENT, fluid_blocks});
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

  for (const auto & var_info : vars_to_add)
    addAuxVariable(var_info.first, var_info.second);
}

void
SubChannelAddVariablesAction::addInitialConditions()
{
  const auto * const quad_mesh = dynamic_cast<const QuadSubChannelMesh *>(_mesh.get());
  const auto * const tri_mesh = dynamic_cast<const TriSubChannelMesh *>(_mesh.get());
  const auto * const subchannel_mesh = quad_mesh ? static_cast<const SubChannelMesh *>(quad_mesh)
                                                 : static_cast<const SubChannelMesh *>(tri_mesh);

  if (!subchannel_mesh)
    mooseError("The SubChannel action requires a SubChannelMesh when adding default ICs.");

  if (quad_mesh)
  {
    addInitialCondition("SCMQuadFlowAreaIC", "subchannel_S_IC", SubChannelApp::SURFACE_AREA);
    addInitialCondition(
        "SCMQuadWettedPerimIC", "subchannel_w_perim_IC", SubChannelApp::WETTED_PERIMETER);
  }
  else
  {
    addInitialCondition("SCMTriFlowAreaIC", "subchannel_S_IC", SubChannelApp::SURFACE_AREA);
    addInitialCondition(
        "SCMTriWettedPerimIC", "subchannel_w_perim_IC", SubChannelApp::WETTED_PERIMETER);
  }

  if (subchannel_mesh->pinMeshExist() && !hasInitialCondition(SubChannelApp::PIN_DIAMETER))
  {
    auto params = _factory.getValidParams("ConstantIC");
    params.set<VariableName>("variable") = SubChannelApp::PIN_DIAMETER;
    params.set<Real>("value") = subchannel_mesh->getPinDiameter();
    _problem->addInitialCondition("ConstantIC", "subchannel_Dpin_IC", params);
  }
}

void
SubChannelAddVariablesAction::addInitialCondition(const std::string & type,
                                                  const std::string & name,
                                                  const VariableName & var_name)
{
  if (hasInitialCondition(var_name))
    return;

  auto params = _factory.getValidParams(type);
  params.set<VariableName>("variable") = var_name;
  _problem->addInitialCondition(type, name, params);
}

bool
SubChannelAddVariablesAction::hasInitialCondition(const VariableName & var_name) const
{
  const auto targets_variable = [&var_name](const MooseObjectAction * action)
  {
    const auto & params = action->getObjectParams();
    return params.isParamValid("variable") && params.get<VariableName>("variable") == var_name;
  };

  for (const auto * action : _awh.getActions<AddInitialConditionAction>())
    if (targets_variable(action))
      return true;

  for (const auto * action : _awh.getActions<AddICAction>())
  {
    if (targets_variable(action))
      return true;

    std::vector<std::string> path;
    MooseUtils::tokenize<std::string>(action->parameters().blockFullpath(), path, 1, "/");
    if (path.size() >= 2 && path[path.size() - 2] == var_name)
      return true;
  }

  return false;
}
